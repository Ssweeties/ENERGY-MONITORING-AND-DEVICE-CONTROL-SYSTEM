/*
-Wait for Firebase "login" trigger
-Init RFID and scan for card
-Validate UID against whitelist
-Load previous energy usage
-Activate relays and start session
 */
#include "main.h"
#include "login.h"
#include "OLED.h"
#include "firebase.h"
#include "RFID.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>

static TaskHandle_t rfid_task_handle = NULL;

bool check_valid_uid(const char *uid) {
    if (!uid || strlen(uid) == 0) return false;
    // Check each character of the scanned UID
    for (int i = 0; VALID_UIDS[i] != NULL; i++) {
        if (strcasecmp(uid, VALID_UIDS[i]) == 0) return true;
    }
    return false;
}

bool login_load_user_energy(const char *uid, float *e1, float *e2) {
    if (!uid || !e1 || !e2) return false;
    // Create a variable to store the Firebase path
    char path[64];
    snprintf(path, sizeof(path), "User/%s/E1", uid);
    bool ok1 = firebase_get_float(path, e1);
    
    snprintf(path, sizeof(path), "User/%s/E2", uid);
    bool ok2 = firebase_get_float(path, e2);

    if (!ok1 || !ok2) {
        *e1 = *e2 = 0;
        return false;
    }
    printf("Loaded: E1=%.3f E2=%.3f Wh", *e1, *e2);
    return true;
}

static void rfid_scan_task(void *pvParameters) {
    if (rfid_init() != ESP_OK) {
        printf("RFID Error");
        firebase_set_bool("login", false);
        rfid_task_handle = NULL;
        vTaskDelete(NULL);
        return;
    }

    char uid[32];
    // Wait for valid card
    while (1) {
        if (rfid_check_new_card() && rfid_read_card_uid(uid, sizeof(uid))) {
            printf("Card: %s", uid);
            // Validate UID
            if (check_valid_uid(uid)) {
                oled_display_ok();
                vTaskDelay(pdMS_TO_TICKS(2000));
                rfid_halt_card(); // Ensure card is halted before break
                break; // Exit loop if uid is valid
            } else {
                printf("Unknown");
            }
            rfid_halt_card();
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    rfid_deinit();
    // Copy the valid UID to the current user uid
    strlcpy(g_system_state.current_uid, uid, sizeof(g_system_state.current_uid));

    float e1 = 0, e2 = 0;
    login_load_user_energy(uid, &e1, &e2);
    g_system_state.dev1.energy = e1;
    g_system_state.dev2.energy = e2;
    firebase_set_float("dev1/E", e1);
    firebase_set_float("dev2/E", e2);

    gpio_set_level(RELAY1_PIN, 1);
    gpio_set_level(RELAY2_PIN, 1);
    g_system_state.relay1_on = true;
    g_system_state.relay2_on = true;
    firebase_set_bool("dev1/Status", true);
    firebase_set_bool("dev2/Status", true);

    g_system_state.is_logged_in = true;
    firebase_set_bool("login", false);
    
    printf("Login complete");
    
    rfid_task_handle = NULL;
    vTaskDelete(NULL);
}

void login_handle(void) {
    if (g_system_state.is_logged_in) {
        printf("Already logged in");
        firebase_set_bool("login", false);
    } else if (rfid_task_handle != NULL) {
        printf("RFID task running");
    } else {
        // Create task and check result
        if (xTaskCreate(rfid_scan_task, "rfid", 8192, NULL, 5, &rfid_task_handle) != pdPASS) {
            printf("Failed to create task");
            firebase_set_bool("login", false);
        }
    }
}