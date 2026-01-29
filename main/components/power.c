/*
- ACS712: Hall-effect current sensor
- ZMPT101B: Voltage transformer
 */
#include "main.h"
#include "power.h"
#include "esp_log.h"
#include <math.h>
#include <stdlib.h>
#include <esp_rom_sys.h>

/* Sensor instances */
ACS712_t g_acs1, g_acs2;
ZMPT101B_t g_zmpt1, g_zmpt2;

// Read ADC
static uint32_t read_adc_mv(adc1_channel_t ch, esp_adc_cal_characteristics_t *chars) {
    uint32_t sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += adc1_get_raw(ch);
    }
    return esp_adc_cal_raw_to_voltage(sum / 10, chars);
}
// ACS712 init
static void acs712_init(ACS712_t *s, adc1_channel_t ch, int sensitivity) {
    s->channel = ch;
    s->sensitivity = sensitivity;
    s->zero_point = 0;
    s->adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH, ADC_VREF, s->adc_chars);
}
// ACS712 calibrate
static void acs712_calibrate(ACS712_t *s) {
    float sum = 0;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        sum += read_adc_mv(s->channel, s->adc_chars);
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    s->zero_point = sum / SAMPLE_COUNT;
    printf("ACS712 ch%d zero=%.0fmV", s->channel, s->zero_point);
}
// ACS712 read
static float acs712_read_ma(ACS712_t *s) {
    float sum_sq = 0;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        uint32_t mv = read_adc_mv(s->channel, s->adc_chars);
        float current = ((float)mv - s->zero_point) / s->sensitivity * 1000.0f;
        sum_sq += current * current;
        // Delay to span one AC cycle (1/AC_FREQUENCY = 20ms)
        // 25 samples * 900us = 22.5ms (covers >1 cycle)
        esp_rom_delay_us(900);  
    }
    return sqrtf(sum_sq / SAMPLE_COUNT);
}
// ZMPT101B init
static void zmpt101b_init(ZMPT101B_t *s, adc1_channel_t ch, float sensitivity) {
    s->channel = ch;
    s->sensitivity = sensitivity;
    s->zero_point = 0;
    s->adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH, ADC_VREF, s->adc_chars);
}
// ZMPT101B calibrate
static void zmpt101b_calibrate(ZMPT101B_t *s) {
    float sum = 0;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        sum += read_adc_mv(s->channel, s->adc_chars);
        vTaskDelay(pdMS_TO_TICKS(2));
    }
    s->zero_point = sum / SAMPLE_COUNT;
    printf("ZMPT ch%d zero=%.0fmV", s->channel, s->zero_point);
}
// ZMPT101B read
static float zmpt101b_read_volt(ZMPT101B_t *s) {
    float sum_sq = 0;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        uint32_t mv = read_adc_mv(s->channel, s->adc_chars);
        float dev = (float)mv - s->zero_point;
        sum_sq += dev * dev;
        esp_rom_delay_us(900);
    }
    float rms = sqrtf(sum_sq / SAMPLE_COUNT);
    float volt = (rms / s->sensitivity) * 220.0f;
    return (volt < 10.0f) ? 0 : volt;  // Noise threshold: 10V
}
// Convert GPIO Pin to ADC1 Channel
static adc1_channel_t get_adc1_channel(int pin) {
    switch (pin) {
        case 36: return ADC1_CHANNEL_0;
        case 37: return ADC1_CHANNEL_1;
        case 38: return ADC1_CHANNEL_2;
        case 39: return ADC1_CHANNEL_3;
        case 32: return ADC1_CHANNEL_4;
        case 33: return ADC1_CHANNEL_5;
        case 34: return ADC1_CHANNEL_6;
        case 35: return ADC1_CHANNEL_7;
        default: return ADC1_CHANNEL_MAX;
    }
}

// Power init
void power_init(void) {
    adc1_config_width(ADC_WIDTH);
    
    // Convert defined PINS to ADC Channels
    adc1_channel_t ACS1 = get_adc1_channel(ACS1_PIN);
    adc1_channel_t ZMPT1 = get_adc1_channel(ZMPT1_PIN);
    adc1_channel_t ACS2 = get_adc1_channel(ACS2_PIN);
    adc1_channel_t ZMPT2 = get_adc1_channel(ZMPT2_PIN);

    // Validate Pins
    if (ACS1 == ADC1_CHANNEL_MAX || ZMPT1 == ADC1_CHANNEL_MAX ||
        ACS2 == ADC1_CHANNEL_MAX || ZMPT2 == ADC1_CHANNEL_MAX) {
        printf("Error: Invalid ADC Pin defined! Check main.h\n");
        return;
    }

    // Configure ADC channels
    adc1_config_channel_atten(ACS1, ADC_ATTEN);
    adc1_config_channel_atten(ZMPT1, ADC_ATTEN);
    adc1_config_channel_atten(ACS2, ADC_ATTEN);
    adc1_config_channel_atten(ZMPT2, ADC_ATTEN);
    
    // Device 1 sensors
    acs712_init(&g_acs1, ACS1, 66);  // 66mV/A for ACS712-30A
    acs712_calibrate(&g_acs1);
    zmpt101b_init(&g_zmpt1, ZMPT1, ZMPT_SENSITIVITY);
    zmpt101b_calibrate(&g_zmpt1);
    
    // Device 2 sensors
    acs712_init(&g_acs2, ACS2, 66);
    acs712_calibrate(&g_acs2);
    zmpt101b_init(&g_zmpt2, ZMPT2, ZMPT_SENSITIVITY);
    zmpt101b_calibrate(&g_zmpt2);
    printf("Sensors Ready");
}
/*
 *   I (current) = (ADC_mV - offset) / sensitivity 
 *   U (voltage) = RMS voltage from sensor 
 *   P (power)   = I × U 
 *   E (energy)  = E + P × dt, where dt = interval in hours
 */
void power_measure(DeviceData_t *d, ACS712_t *acs, ZMPT101B_t *zmpt) {
    if (!d || !acs || !zmpt) return;
    
    // Read current (mA) and convert to Amperes
    float i_ma = acs712_read_ma(acs);
    d->current = (i_ma - CALIBRATION_FACTOR) / 1000.0f;
    if (d->current < 0.05f) d->current = 0;  // Noise threshold: 50mA
    
    // Read voltage (Volts)
    d->voltage = zmpt101b_read_volt(zmpt);
    
    // Calculate power: P = V × I
    d->power = d->current * d->voltage;
    
    // Accumulate energy: E += P × (ms / 3600000) to get Wh
    d->energy += d->power * (MEASURE_INTERVAL_MS / 3600000.0f);
}