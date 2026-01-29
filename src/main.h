#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define WIFI_SSID           "Redmi Note 14" //Name Wifi
#define WIFI_PASSWORD       "30112004"      //Password Wifi

#define FIREBASE_API_KEY    "AIzaSyDQUcoM_JiDbA1UcZ6bKlqztVYQ1uaiyHg"
#define FIREBASE_DB_URL     "https://do-an-2-a0cea-default-rtdb.firebaseio.com/"

#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       64
#define OLED_I2C_ADDR       0x3C
#define OLED_SDA_PIN        21
#define OLED_SCL_PIN        22

#define AC_FREQUENCY        50.0f       // AC frequency (Hz)
#define ZMPT_SENSITIVITY    405.5f      // ZMPT101B voltage sensor sensitivity
#define CALIBRATION_FACTOR  120         // Current sensor offset
#define MEASURE_INTERVAL_MS 1500        // Measurement interval (ms)

#define ACS1_PIN            34          
#define ZMPT1_PIN           35          

#define ACS2_PIN            32          
#define ZMPT2_PIN           33          

#define RELAY1_PIN          14
#define RELAY2_PIN          27

#define RFID_SS_PIN         5
#define RFID_RST_PIN        4
extern const char* VALID_UIDS[];  // NULL-terminated array

typedef struct {
    float current;      
    float voltage;      
    float power;        
    float energy;       
} DeviceData_t;

typedef struct {
    volatile bool is_logged_in;         
    volatile bool logout_requested;     
    volatile bool login_requested;      
    volatile bool is_wifi_connected;
    volatile bool relay1_on;
    volatile bool relay2_on;
    char current_uid[32];               
    DeviceData_t dev1;
    DeviceData_t dev2;
    uint32_t last_measure_time;         
} SystemState_t;

extern SystemState_t g_system_state;
extern SemaphoreHandle_t g_firebase_mutex;

#define FB_LOCK()   xSemaphoreTake(g_firebase_mutex, portMAX_DELAY)
#define FB_UNLOCK() xSemaphoreGive(g_firebase_mutex)

void system_init(void);

#endif