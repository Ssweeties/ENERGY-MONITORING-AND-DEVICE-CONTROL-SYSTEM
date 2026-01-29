#ifndef OLED_H
#define OLED_H

#include <stdint.h>
#include "driver/i2c.h"
#include "esp_err.h"

/* I2C Configuration */
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          400000
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_MASTER_TIMEOUT_MS       1000

/* SSD1306 Commands */
#define SSD1306_CMD_DISPLAY_OFF             0xAE
#define SSD1306_CMD_DISPLAY_ON              0xAF
#define SSD1306_CMD_SET_DISPLAY_CLOCK_DIV   0xD5
#define SSD1306_CMD_SET_MULTIPLEX           0xA8
#define SSD1306_CMD_SET_DISPLAY_OFFSET      0xD3
#define SSD1306_CMD_SET_START_LINE          0x40
#define SSD1306_CMD_CHARGE_PUMP             0x8D
#define SSD1306_CMD_MEMORY_MODE             0x20
#define SSD1306_CMD_SEG_REMAP               0xA1
#define SSD1306_CMD_COM_SCAN_DEC            0xC8
#define SSD1306_CMD_SET_COM_PINS            0xDA
#define SSD1306_CMD_SET_CONTRAST            0x81
#define SSD1306_CMD_SET_PRECHARGE           0xD9
#define SSD1306_CMD_SET_VCOM_DETECT         0xDB
#define SSD1306_CMD_DISPLAY_ALL_ON_RESUME   0xA4
#define SSD1306_CMD_NORMAL_DISPLAY          0xA6
#define SSD1306_CMD_COLUMN_ADDR             0x21
#define SSD1306_CMD_PAGE_ADDR               0x22
#define SSD1306_CMD_DEACTIVATE_SCROLL       0x2E

#define OLED_CONTROL_BYTE_CMD_SINGLE        0x80
#define OLED_CONTROL_BYTE_DATA_STREAM       0x40

typedef struct {
    uint8_t address;        // I2C address (0x3C)
    uint8_t width;          // Display width (128)
    uint8_t height;         // Display height (64)
    uint8_t *buffer;        // Framebuffer pointer
    uint16_t buffer_size;   // Buffer size in bytes (1024)
} OLED_t;

extern OLED_t g_oled;
extern const uint8_t font5x7[][5];

esp_err_t oled_init(void);

void oled_clear_buffer(void);

void oled_push_buffer(void);

void oled_draw_pixel(int16_t x, int16_t y, uint8_t color);

void oled_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color);

void oled_draw_char(int16_t x, int16_t y, char c, uint8_t size);

void oled_draw_string(int16_t x, int16_t y, const char *str, uint8_t size);

/* Display login request screen */
void oled_display_please_login(void);

/* Display connect wifi screen */
void oled_display_connect_wifi(void);

/* Display OK screen */
void oled_display_ok(void);

/* Display power monitoring layout */
void oled_draw_layout(void *dev1, void *dev2);

#endif