#ifndef POWER_H
#define POWER_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define ADC_WIDTH       ADC_WIDTH_BIT_12    // 12-bit resolution (0-4095)
#define ADC_ATTEN       ADC_ATTEN_DB_12     // 11dB attenuation (0-3.3V range)
#define ADC_VREF        3300                // Reference voltage in mV
#define ADC_MAX_VALUE   4095
#define SAMPLE_COUNT    25                  // Samples per AC cycle

typedef struct {
    adc1_channel_t channel;
    int sensitivity;                        // mV/A
    float zero_point;                       // Calibrated zero offset (mV)
    esp_adc_cal_characteristics_t *adc_chars;
} ACS712_t;

typedef struct {
    adc1_channel_t channel;
    float sensitivity;
    float zero_point;
    esp_adc_cal_characteristics_t *adc_chars;
} ZMPT101B_t;

extern ACS712_t g_acs1, g_acs2;
extern ZMPT101B_t g_zmpt1, g_zmpt2;

void power_init(void);

void power_measure(DeviceData_t *d, ACS712_t *acs, ZMPT101B_t *zmpt);

#endif