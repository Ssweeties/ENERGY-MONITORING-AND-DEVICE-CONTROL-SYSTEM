/*
 RFID.c - MFRC522 RFID Reader Driver
 RC522 via SPI. Supports ISO/IEC 14443 Type A (MIFARE).
 */

#include "main.h"
#include "RFID.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>

static spi_device_handle_t spi_handle = NULL;
static char last_uid[32] = "";
static uint32_t last_seen_time = 0;
// SPI init
static void rfid_spi_init(void) {

    spi_bus_config_t bus_cfg = {
        .miso_io_num = 19,
        .mosi_io_num = 23,
        .sclk_io_num = 18,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
    };

    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 1000000,
        .mode = 0,
        .spics_io_num = RFID_SS_PIN,
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev_cfg, &spi_handle));
    printf("SPI Ready");
}
/*
 * Write to MFRC522 register via SPI
 * 
 * SPI Frame format:
 *   Byte 0: Address byte = (reg << 1) & 0x7E
 *           - Bit 7 = 0 (write operation)
 *           - Bits 6:1 = register address
 *           - Bit 0 = 0 (reserved)
 *   Byte 1: Data to write
 */
void rfid_write_register(uint8_t reg, uint8_t value) {
    uint8_t tx_data[2] = { (reg << 1) & 0x7E, value };
    spi_transaction_t trans = { .length = 16, .tx_buffer = tx_data };
    spi_device_transmit(spi_handle, &trans);
}

/*
 * Read from MFRC522 register via SPI
 * 
 * SPI Frame format:
 *   Byte 0: Address byte = ((reg << 1) & 0x7E) | 0x80
 *           - Bit 7 = 1 (read operation)
 *           - Bits 6:1 = register address
 *           - Bit 0 = 0 (reserved)
 *   Byte 1: Dummy byte (0x00), clock out data from chip
 * Return: Data read from register
 */
uint8_t rfid_read_register(uint8_t reg) {
    uint8_t tx_data[2] = { ((reg << 1) & 0x7E) | 0x80, 0x00 };
    uint8_t rx_data[2];
    spi_transaction_t trans = { .length = 16, .tx_buffer = tx_data, .rx_buffer = rx_data };
    spi_device_transmit(spi_handle, &trans);
    return rx_data[1];
}
// Send command
void rfid_send_command(uint8_t command) {
    rfid_write_register(MFRC522_REG_COMMAND, command);
}
// Reset
void rfid_reset(void) {
    rfid_send_command(MFRC522_CMD_SOFT_RESET);
    vTaskDelay(pdMS_TO_TICKS(50));
}
// Antenna on
void rfid_antenna_on(void) {
    uint8_t value = rfid_read_register(MFRC522_REG_TX_CONTROL);
    if ((value & 0x03) != 0x03) {
        rfid_write_register(MFRC522_REG_TX_CONTROL, value | 0x03);
    }
}
// Antenna off
void rfid_antenna_off(void) {
    uint8_t value = rfid_read_register(MFRC522_REG_TX_CONTROL);
    rfid_write_register(MFRC522_REG_TX_CONTROL, value & ~0x03);
}
// RFID Init
esp_err_t rfid_init(void) {
    gpio_config_t rst_cfg = {
        .pin_bit_mask = (1ULL << RFID_RST_PIN),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&rst_cfg);

    // Hardware reset
    gpio_set_level(RFID_RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(RFID_RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(RFID_RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(50));

    rfid_spi_init();
    rfid_reset();

    // Configure MFRC522
    rfid_write_register(MFRC522_REG_T_MODE, 0x8D);
    rfid_write_register(MFRC522_REG_T_PRESCALER, 0x3E);
    rfid_write_register(MFRC522_REG_T_RELOAD_L, 30);
    rfid_write_register(MFRC522_REG_T_RELOAD_H, 0);
    rfid_write_register(MFRC522_REG_TX_ASK, 0x40);
    rfid_write_register(MFRC522_REG_MODE, 0x3D);

    rfid_antenna_on();

    uint8_t version = rfid_read_register(MFRC522_REG_VERSION);

    if (version == 0x00 || version == 0xFF) {
        printf("RFID error");
        return ESP_FAIL;
    }

    printf("RFID Ready");
    return ESP_OK;
}
// RFID Deinit
void rfid_deinit(void) {
    rfid_antenna_off();
    if (spi_handle != NULL) {
        spi_bus_remove_device(spi_handle);
        spi_bus_free(HSPI_HOST);
        spi_handle = NULL;
    }
    printf("RFID Deinitialized");
}
// Wait for IRQ
static bool rfid_wait_for_irq(uint8_t irq_mask, uint32_t timeout_ms) {
    uint32_t start = xTaskGetTickCount() * portTICK_PERIOD_MS;
    while (1) {
        uint8_t irq = rfid_read_register(MFRC522_REG_COM_IRQ);
        if (irq & irq_mask) return true;
        if (irq & 0x01) return false;
        if ((xTaskGetTickCount() * portTICK_PERIOD_MS - start) > timeout_ms) return false;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
// Check new card
bool rfid_check_new_card(void) {
    rfid_write_register(MFRC522_REG_COM_IRQ, 0x7F);
    rfid_send_command(MFRC522_CMD_IDLE);
    rfid_write_register(MFRC522_REG_FIFO_LEVEL, 0x80);
    rfid_write_register(MFRC522_REG_FIFO_DATA, PICC_CMD_REQA);
    rfid_write_register(MFRC522_REG_BIT_FRAMING, 0x07);
    rfid_send_command(MFRC522_CMD_TRANSCEIVE);
    rfid_write_register(MFRC522_REG_BIT_FRAMING, rfid_read_register(MFRC522_REG_BIT_FRAMING) | 0x80);
    
    if (!rfid_wait_for_irq(0x30, 50)) return false;

    if (rfid_read_register(MFRC522_REG_ERROR) & 0x13) return false;

    return rfid_read_register(MFRC522_REG_FIFO_LEVEL) > 0;
}
// Read card UID
bool rfid_read_card_uid(char *uid_buffer, size_t buffer_size) {
    if (!uid_buffer || buffer_size < 12) return false;

    rfid_uid_t uid = {0};

    rfid_write_register(MFRC522_REG_COM_IRQ, 0x7F);
    rfid_send_command(MFRC522_CMD_IDLE);
    rfid_write_register(MFRC522_REG_FIFO_LEVEL, 0x80);
    rfid_write_register(MFRC522_REG_FIFO_DATA, PICC_CMD_SEL_CL1);
    rfid_write_register(MFRC522_REG_FIFO_DATA, 0x20);
    rfid_write_register(MFRC522_REG_BIT_FRAMING, 0x00);
    
    rfid_send_command(MFRC522_CMD_TRANSCEIVE); rfid_write_register(MFRC522_REG_BIT_FRAMING, rfid_read_register(MFRC522_REG_BIT_FRAMING) | 0x80);
    
    if (!rfid_wait_for_irq(0x30, 50)) return false;
    if (rfid_read_register(MFRC522_REG_ERROR) & 0x13) return false;
    if (rfid_read_register(MFRC522_REG_FIFO_LEVEL) != 5) return false;
    
    for (int i = 0; i < 4; i++) {
        uid.uid_byte[i] = rfid_read_register(MFRC522_REG_FIFO_DATA);
    }
    uid.size = 4;
    
    snprintf(uid_buffer, buffer_size, "%02X:%02X:%02X:%02X",
             uid.uid_byte[0], uid.uid_byte[1], uid.uid_byte[2], uid.uid_byte[3]);
    
    // Debounce
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    if (strcmp(uid_buffer, last_uid) == 0 && (now - last_seen_time) < RFID_DEBOUNCE_TIME_MS) {
        return false;
    }
    strncpy(last_uid, uid_buffer, sizeof(last_uid) - 1);
    last_seen_time = now;
    
    return true;
}
// Halt card
void rfid_halt_card(void) {
    rfid_write_register(MFRC522_REG_FIFO_LEVEL, 0x80);
    rfid_write_register(MFRC522_REG_FIFO_DATA, PICC_CMD_HLTA);
    rfid_write_register(MFRC522_REG_FIFO_DATA, 0x00);
    rfid_send_command(MFRC522_CMD_TRANSCEIVE);
    vTaskDelay(pdMS_TO_TICKS(5));
}