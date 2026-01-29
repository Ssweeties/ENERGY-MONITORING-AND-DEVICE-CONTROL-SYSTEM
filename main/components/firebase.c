/*
HTTP REST API to communicate with Firebase:
-PUT: Write single value (set_float, set_bool)
-PATCH: Update multiple values at once (patch_json)
-GET: Read value (get_float, read_streams)
Thread safety: All operations protected by mutex (FB_LOCK/FB_UNLOCK)
 */
#include "../main.h"
#include "firebase.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h" 
#include <string.h>
#include <stdlib.h>

// Buffer sizes for Firebase operations
#define FB_URL_BUFFER_SIZE      768
#define FB_JSON_BUFFER_SIZE     256
#define FB_STREAM_BUFFER_SIZE   1024

static char firebase_url[256];
static char firebase_auth[256];
// Firebase init
void firebase_init(void) {
    esp_log_level_set("esp-x509-crt-bundle", ESP_LOG_NONE);
    // Copy Firebase URL to local variable
    strlcpy(firebase_url, FIREBASE_DB_URL, sizeof(firebase_url));
    size_t len = strlen(firebase_url);
    // Remove trailing slash
    if (len > 0 && firebase_url[len - 1] == '/') {
        firebase_url[len - 1] = '\0';
    }
    
    snprintf(firebase_auth, sizeof(firebase_auth), "?auth=%s", FIREBASE_API_KEY);
    printf("FB Ready");
}
// Set float
void firebase_set_float(const char *path, float value) {
    FB_LOCK();
    
    char url[FB_URL_BUFFER_SIZE];
    char data[32];
    // Format URL and data
    snprintf(url, sizeof(url), "%s/%s.json%s", firebase_url, path, firebase_auth);
    snprintf(data, sizeof(data), "%.6f", value);
    
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_PUT,
        .timeout_ms = 10000,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .skip_cert_common_name_check = true,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, data, strlen(data));
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
    
    FB_UNLOCK();
}
// Set bool
void firebase_set_bool(const char *path, bool value) {
    FB_LOCK();
    // Format URL and data
    char url[FB_URL_BUFFER_SIZE];
    snprintf(url, sizeof(url), "%s/%s.json%s", firebase_url, path, firebase_auth);
    const char *data = value ? "true" : "false";
    
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_PUT,
        .timeout_ms = 10000,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .skip_cert_common_name_check = true,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, data, strlen(data));
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
    
    FB_UNLOCK();
}
// Patch JSON
void firebase_patch_json(const char *path, const char *json) {
    FB_LOCK();
    
    char url[FB_URL_BUFFER_SIZE];
    // Format URL and data
    if (path && *path) {
        snprintf(url, sizeof(url), "%s/%s.json%s", firebase_url, path, firebase_auth);
    } else {
        snprintf(url, sizeof(url), "%s/.json%s", firebase_url, firebase_auth);
    }
    
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_PATCH,
        .timeout_ms = 10000,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .skip_cert_common_name_check = true,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json, strlen(json));
    esp_http_client_perform(client);
    esp_http_client_cleanup(client);
    
    FB_UNLOCK();
}
// Get float
bool firebase_get_float(const char *path, float *output) {
    if (!path || !output) return false;
    
    FB_LOCK();
    
    char url[FB_URL_BUFFER_SIZE];
    char buffer[64] = {0};
    snprintf(url, sizeof(url), "%s/%s.json%s", firebase_url, path, firebase_auth);
    
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 10000,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .skip_cert_common_name_check = true,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_open(client, 0);
    
    bool success = false;
    if (err == ESP_OK) {
        int len = esp_http_client_fetch_headers(client);
        if (len > 0 && len < (int)sizeof(buffer)) {
            esp_http_client_read(client, buffer, len);
            // Convert string to float and update output value
            *output = strtof(buffer, NULL);
            success = true;
        }
    }
    
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    FB_UNLOCK();
    return success;
}
// Read streams
void firebase_read_streams(void) {
    FB_LOCK();
    
    char url[FB_URL_BUFFER_SIZE];
    char buffer[FB_STREAM_BUFFER_SIZE] = {0};
    snprintf(url, sizeof(url), "%s/.json%s", firebase_url, firebase_auth);
    
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 15000,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .skip_cert_common_name_check = true,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_open(client, 0);
    
    if (err == ESP_OK) {
        int len = esp_http_client_fetch_headers(client);
        if (len > 0 && len < (int)sizeof(buffer)) {
            esp_http_client_read(client, buffer, len);
            
            char *ptr;
            // Check for login from firebase
            ptr = strstr(buffer, "\"login\":");
            if (ptr) {
                bool val = (strstr(ptr, "true") != NULL && strstr(ptr, "true") < ptr + 20);
                if (val && !g_system_state.login_requested) {
                    printf("Login detected");
                }
                g_system_state.login_requested = val;
            }
            // Check for logout from firebase
            ptr = strstr(buffer, "\"logout\":");
            if (ptr) {
                bool val = (strstr(ptr, "true") != NULL && strstr(ptr, "true") < ptr + 20);
                if (val && !g_system_state.logout_requested) {
                    printf("Logout detected");
                }
                g_system_state.logout_requested = val;
            }
            // Check for devices status from firebase
            ptr = strstr(buffer, "\"dev1\":");
            if (ptr) {
                char *status = strstr(ptr, "\"Status\":");
                if (status && status < ptr + 200) {
                    g_system_state.relay1_on = (strstr(status, "true") != NULL && strstr(status, "true") < status + 20);
                }
            }
            
            ptr = strstr(buffer, "\"dev2\":");
            if (ptr) {
                char *status = strstr(ptr, "\"Status\":");
                if (status && status < ptr + 200) {
                    g_system_state.relay2_on = (strstr(status, "true") != NULL && strstr(status, "true") < status + 20);
                }
            }
        }
    }
    
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    
    FB_UNLOCK();
}
// Update data
void firebase_update_data(void) {
    char json[256];
    //sprintf(char *str, const char *format, ...)
    snprintf(json, sizeof(json), 
        "{\"dev1/I\":%.3f,\"dev1/U\":%.1f,\"dev1/P\":%.2f,\"dev1/E\":%.3f,"
        "\"dev2/I\":%.3f,\"dev2/U\":%.1f,\"dev2/P\":%.2f,\"dev2/E\":%.3f}",
        g_system_state.dev1.current, g_system_state.dev1.voltage,
        g_system_state.dev1.power, g_system_state.dev1.energy,
        g_system_state.dev2.current, g_system_state.dev2.voltage,
        g_system_state.dev2.power, g_system_state.dev2.energy);
    firebase_patch_json("", json);
}
// Store user energy
void firebase_store_user_energy(const char *uid, float e1, float e2) {
    if (!uid || !*uid) return;
    
    char path[64];
    
    snprintf(path, sizeof(path), "User/%s/E1", uid);
    firebase_set_float(path, e1);
    
    snprintf(path, sizeof(path), "User/%s/E2", uid);
    firebase_set_float(path, e2);
    
    snprintf(path, sizeof(path), "User/%s/E", uid);
    firebase_set_float(path, e1 + e2);
}