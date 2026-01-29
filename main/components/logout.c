/*
-Save energy data to Firebase
-Reset measurements and session
-Deactivate relays
-Return to login screen
 */

#include "main.h"
#include "logout.h"
#include "OLED.h"
#include "firebase.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>

void logout_handle(void) {
    // Snapshot current session
    char uid[32];
    strlcpy(uid, g_system_state.current_uid, sizeof(uid));
    float e1 = g_system_state.dev1.energy;
    float e2 = g_system_state.dev2.energy;
    
    printf("Saving: UID=%s E1=%.3f E2=%.3f Wh", uid, e1, e2);
    
    // Save to Firebase
    if (strlen(uid) > 0) {
        firebase_store_user_energy(uid, e1, e2);
    }
    
    // Clear session
    g_system_state.is_logged_in = false;
    memset(g_system_state.current_uid, 0, sizeof(g_system_state.current_uid));
    
    // Reset measurements
    g_system_state.dev1.current = 0;
    g_system_state.dev1.voltage = 0;
    g_system_state.dev1.power = 0;
    g_system_state.dev1.energy = 0;

    g_system_state.dev2.current = 0;
    g_system_state.dev2.voltage = 0;
    g_system_state.dev2.power = 0;
    g_system_state.dev2.energy = 0;
    
    firebase_set_float("dev1/E", 0);
    firebase_set_float("dev2/E", 0);
    firebase_update_data();
    
    // Turn off relays
    gpio_set_level(RELAY1_PIN, 0);
    gpio_set_level(RELAY2_PIN, 0);
    g_system_state.relay1_on = false;
    g_system_state.relay2_on = false;
    firebase_set_bool("dev1/Status", false);
    firebase_set_bool("dev2/Status", false);
    
    oled_display_please_login();
    firebase_set_bool("logout", false);
    
    printf("Logout complete");
}