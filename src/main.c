/*
ENERGY MONITORING AND DEVICE CONTROL SYSTEM
FEATURES:
-Monitor electrical values (V, I, P, E)
-Control devices via Relays
-Sync data with Firebase and OLED
-RFID authentication
WORKFLOW:
Login -> Active Relays -> Measure -> Sync -> Logout
 */

#include "main.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "../components/power.h"
#include "../components/OLED.h"
#include "../components/login.h"
#include "../components/logout.h"
#include "../components/firebase.h"

const char* VALID_UIDS[] = {
    "EE:E0:E8:00",
    "0A:4E:17:05",
};
// System state
SystemState_t g_system_state = {
    .is_logged_in = false,
    .logout_requested = false,
    .login_requested = false,
    .is_wifi_connected = false,
    .current_uid = "",
    .dev1 = {0, 0, 0, 0},
    .dev2 = {0, 0, 0, 0},
    .last_measure_time = 0
};
// Firebase mutex
SemaphoreHandle_t g_firebase_mutex = NULL;

// GPIO initialization
void gpio_init(void) {
    gpio_config_t relay_conf = {
        .pin_bit_mask = (1ULL << RELAY1_PIN) | (1ULL << RELAY2_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&relay_conf);
    gpio_set_level(RELAY1_PIN, 0);
    gpio_set_level(RELAY2_PIN, 0);
    printf("GPIO initialized");
}
// WiFi event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        printf("WiFi disconnected, reconnecting...");
        g_system_state.is_wifi_connected = false;
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("WiFi connected, IP: " IPSTR, IP2STR(&event->ip_info.ip));
        g_system_state.is_wifi_connected = true;
    }
}
// WiFi init
void wifi_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id, instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                    &wifi_event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                    &wifi_event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = { .ssid = WIFI_SSID, .password = WIFI_PASSWORD },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}
// System init
void system_init(void) {
    // NVS Flash
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Mutex for Firebase thread safety
    g_firebase_mutex = xSemaphoreCreateMutex();
    if (g_firebase_mutex == NULL) {
        printf("Failed to create mutex");
        return;
    }

    gpio_init();
    oled_init();
    oled_display_connect_wifi();
    wifi_init();
    
    printf("Waiting for WiFi...");
    vTaskDelay(pdMS_TO_TICKS(5000));

    if (g_system_state.is_wifi_connected) {
        oled_display_please_login();
    }
    power_init();
    firebase_init();

    printf("System ready");
}
// Firebase task handle
TaskHandle_t g_firebase_task_handle = NULL;

// Listen for changes from Firebase
void firebase_stream_task(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(2000));    
    while (1) {
        // Handle stream data (blocking read)
        firebase_read_streams();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Firebase task
void firebase_task(void *pvParameters) {
    static uint32_t last_user_push = 0;
    while (1) {
        // Wait for next measurement
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        if (g_system_state.is_logged_in) {
            // Push data to Firebase
            firebase_update_data();
            
            // Store user energy every 5s
            uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
            if (now - last_user_push >= 5000 && strlen(g_system_state.current_uid) > 0) {
                firebase_store_user_energy(g_system_state.current_uid,
                                         g_system_state.dev1.energy,
                                         g_system_state.dev2.energy);
                last_user_push = now;
            }
        } 
    }
}
// Main task
void main_task(void *pvParameters) {
    while (1) {
        // Not Logged In
        if (!g_system_state.is_logged_in) {
            if (g_system_state.login_requested) {
                // Start RFID scan
                g_system_state.login_requested = false;
                login_handle();
            } else {
                // Display connectivity status
                if (!g_system_state.is_wifi_connected) {
                    oled_display_connect_wifi();
                } else {
                    oled_display_please_login();
                }
            }
        }
        // Logged In
        else {
            // Control relays
            gpio_set_level(RELAY1_PIN, g_system_state.relay1_on ? 1 : 0);
            gpio_set_level(RELAY2_PIN, g_system_state.relay2_on ? 1 : 0);
            
            // Periodic measurements (1.5s)
            uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
            if (now - g_system_state.last_measure_time >= MEASURE_INTERVAL_MS) {
                g_system_state.last_measure_time = now;
                
                // Read active sensors
                if (g_system_state.relay1_on) {
                    power_measure(&g_system_state.dev1, &g_acs1, &g_zmpt1);
                } else {
                    g_system_state.dev1.current = g_system_state.dev1.voltage = g_system_state.dev1.power = 0;
                }
                
                if (g_system_state.relay2_on) {
                    power_measure(&g_system_state.dev2, &g_acs2, &g_zmpt2);
                } else {
                    g_system_state.dev2.current = g_system_state.dev2.voltage = g_system_state.dev2.power = 0;
                }

                // Update display and cloud
                oled_draw_layout(&g_system_state.dev1, &g_system_state.dev2);
                
                if (g_firebase_task_handle != NULL) {
                    xTaskNotifyGive(g_firebase_task_handle);
                }
            }
            
            // Check logout
            if (g_system_state.logout_requested) {
                g_system_state.logout_requested = false;
                logout_handle();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void) {
    system_init();
    xTaskCreate(firebase_stream_task, "fb_stream", 8192, NULL, 4, NULL);
    xTaskCreate(firebase_task, "fb_task", 8192, NULL, 4, &g_firebase_task_handle);
    xTaskCreate(main_task, "main_task", 40960, NULL, 5, NULL);
}