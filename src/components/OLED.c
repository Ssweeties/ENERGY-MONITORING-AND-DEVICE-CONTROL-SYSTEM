/*
-128x64 pixel display via I2C
-5x7 ASCII bitmap font
 */
#include "main.h"
#include "OLED.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

OLED_t g_oled;
// 5x7 ASCII font
const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // backslash
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // f
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z
    {0x00, 0x08, 0x36, 0x41, 0x00}, // {
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // |
    {0x00, 0x41, 0x36, 0x08, 0x00}, // }
    {0x10, 0x08, 0x08, 0x10, 0x08}, // ~
};

esp_err_t i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = OLED_SDA_PIN,
        .scl_io_num = OLED_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) return err;

    return i2c_driver_install(I2C_MASTER_NUM, conf.mode,
                              I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);
}

void oled_write_command(uint8_t cmd) {
    i2c_cmd_handle_t i2c_cmd = i2c_cmd_link_create();
    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (g_oled.address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(i2c_cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
    i2c_master_write_byte(i2c_cmd, cmd, true);
    i2c_master_stop(i2c_cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, i2c_cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(i2c_cmd);
}

void oled_write_data(uint8_t *data, size_t len) {
    i2c_cmd_handle_t i2c_cmd = i2c_cmd_link_create();
    i2c_master_start(i2c_cmd);
    i2c_master_write_byte(i2c_cmd, (g_oled.address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(i2c_cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
    i2c_master_write(i2c_cmd, data, len, true);
    i2c_master_stop(i2c_cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, i2c_cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(i2c_cmd);
}

esp_err_t oled_init(void) {
    esp_err_t err = i2c_master_init();
    if (err != ESP_OK) return err;

    g_oled.address = OLED_I2C_ADDR;
    g_oled.width = SCREEN_WIDTH;
    g_oled.height = SCREEN_HEIGHT;
    g_oled.buffer_size = (SCREEN_WIDTH * SCREEN_HEIGHT) / 8; //(128*64)/8 = 1024 byte

    g_oled.buffer = (uint8_t *)calloc(g_oled.buffer_size, sizeof(uint8_t));
    if (g_oled.buffer == NULL) {
        printf("Buffer alloc failed");
        return ESP_ERR_NO_MEM;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    // SSD1306 init sequence
    oled_write_command(SSD1306_CMD_DISPLAY_OFF);
    oled_write_command(SSD1306_CMD_SET_DISPLAY_CLOCK_DIV);
    oled_write_command(0x80);
    oled_write_command(SSD1306_CMD_SET_MULTIPLEX);
    oled_write_command(SCREEN_HEIGHT - 1);
    oled_write_command(SSD1306_CMD_SET_DISPLAY_OFFSET);
    oled_write_command(0x00);
    oled_write_command(SSD1306_CMD_SET_START_LINE | 0x00);
    oled_write_command(SSD1306_CMD_CHARGE_PUMP);
    oled_write_command(0x14);
    oled_write_command(SSD1306_CMD_MEMORY_MODE);
    oled_write_command(0x00);
    oled_write_command(SSD1306_CMD_SEG_REMAP);
    oled_write_command(SSD1306_CMD_COM_SCAN_DEC);
    oled_write_command(SSD1306_CMD_SET_COM_PINS);
    oled_write_command(0x12);
    oled_write_command(SSD1306_CMD_SET_CONTRAST);
    oled_write_command(0xCF);
    oled_write_command(SSD1306_CMD_SET_PRECHARGE);
    oled_write_command(0xF1);
    oled_write_command(SSD1306_CMD_SET_VCOM_DETECT);
    oled_write_command(0x40);
    oled_write_command(SSD1306_CMD_DISPLAY_ALL_ON_RESUME);
    oled_write_command(SSD1306_CMD_NORMAL_DISPLAY);
    oled_write_command(SSD1306_CMD_DEACTIVATE_SCROLL);
    oled_write_command(SSD1306_CMD_DISPLAY_ON);
    oled_clear_buffer();
    oled_push_buffer();

    printf("OLED Ready");
    return ESP_OK;
}

void oled_clear_buffer(void) {
    // Set 1024 bytes in the buffer to 0
    memset(g_oled.buffer, 0, g_oled.buffer_size);
}

void oled_push_buffer(void) {
    oled_write_command(SSD1306_CMD_COLUMN_ADDR);
    oled_write_command(0);
    oled_write_command(SCREEN_WIDTH - 1);
    oled_write_command(SSD1306_CMD_PAGE_ADDR);
    oled_write_command(0);
    oled_write_command((SCREEN_HEIGHT / 8) - 1);
    oled_write_data(g_oled.buffer, g_oled.buffer_size);
}

void oled_draw_pixel(int16_t x, int16_t y, uint8_t color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) return;
    
    if (color) {
        g_oled.buffer[x + (y / 8) * SCREEN_WIDTH] |= (1 << (y % 8));
    } else {
        g_oled.buffer[x + (y / 8) * SCREEN_WIDTH] &= ~(1 << (y % 8));
    }
}

void oled_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color) {
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    while (1) {
        oled_draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

void oled_draw_char(int16_t x, int16_t y, char c, uint8_t size) {
    if (c < 32 || c > 126) return;
    
    const uint8_t *glyph = font5x7[c - 32];
    
    for (uint8_t i = 0; i < 5; i++) {
        uint8_t line = glyph[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (line & 0x01) {
                if (size == 1) {
                    oled_draw_pixel(x + i, y + j, 1);
                } else {
                    for (uint8_t a = 0; a < size; a++) {
                        for (uint8_t b = 0; b < size; b++) {
                            oled_draw_pixel(x + i * size + a, y + j * size + b, 1);
                        }
                    }
                }
            }
            line >>= 1;
        }
    }
}

void oled_draw_string(int16_t x, int16_t y, const char *str, uint8_t size) {
    int16_t cursor_x = x;
    while (*str) {
        oled_draw_char(cursor_x, y, *str, size);
        cursor_x += 6 * size;
        str++;
    }
}

void oled_display_please_login(void) {
    oled_clear_buffer();
    oled_draw_string(28, 18, "PLEASE", 2);
    oled_draw_string(34, 40, "LOGIN", 2);
    oled_push_buffer();
}

void oled_display_connect_wifi(void) {
    oled_clear_buffer();
    oled_draw_string(22, 18, "CONNECT", 2);
    oled_draw_string(22, 40, "WIFI...", 2);
    oled_push_buffer();
}

void oled_display_ok(void) {
    oled_clear_buffer();
    oled_draw_string(52, 25, "OK", 2);
    oled_push_buffer();
}

void oled_draw_layout(void *dev1, void *dev2) {
    if (!dev1 || !dev2) return;

    DeviceData_t *d1 = (DeviceData_t *)dev1;
    DeviceData_t *d2 = (DeviceData_t *)dev2;

    oled_clear_buffer();

    int divider_x = SCREEN_WIDTH / 2;

    oled_draw_string(5, 2, "Device 1", 1);
    oled_draw_string(divider_x + 4, 2, "Device 2", 1);
    oled_draw_line(divider_x, 0, divider_x, SCREEN_HEIGHT - 1, 1);

    int base_y = 18;
    int dy = 12;
    char buf[20];

    // Device 1
    snprintf(buf, sizeof(buf), "I=%.3fA", d1->current);
    oled_draw_string(4, base_y, buf, 1);
    snprintf(buf, sizeof(buf), "U=%.1fV", d1->voltage);
    oled_draw_string(4, base_y + dy, buf, 1);
    snprintf(buf, sizeof(buf), "P=%.2fW", d1->power);
    oled_draw_string(4, base_y + 2 * dy, buf, 1);
    snprintf(buf, sizeof(buf), "E=%.2fWh", d1->energy);
    oled_draw_string(4, base_y + 3 * dy, buf, 1);

    // Device 2
    int x_right = divider_x + 4;
    snprintf(buf, sizeof(buf), "I=%.3fA", d2->current);
    oled_draw_string(x_right, base_y, buf, 1);
    snprintf(buf, sizeof(buf), "U=%.1fV", d2->voltage);
    oled_draw_string(x_right, base_y + dy, buf, 1);
    snprintf(buf, sizeof(buf), "P=%.2fW", d2->power);
    oled_draw_string(x_right, base_y + 2 * dy, buf, 1);
    snprintf(buf, sizeof(buf), "E=%.2fWh", d2->energy);
    oled_draw_string(x_right, base_y + 3 * dy, buf, 1);

    oled_push_buffer();
}