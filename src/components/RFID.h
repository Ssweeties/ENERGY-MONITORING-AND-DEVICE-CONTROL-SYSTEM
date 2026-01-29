#ifndef RFID_H
#define RFID_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/* MFRC522 Commands */
#define MFRC522_CMD_IDLE            0x00
#define MFRC522_CMD_MEM             0x01
#define MFRC522_CMD_CALC_CRC        0x03
#define MFRC522_CMD_TRANSMIT        0x04
#define MFRC522_CMD_NO_CMD_CHANGE   0x07
#define MFRC522_CMD_RECEIVE         0x08
#define MFRC522_CMD_TRANSCEIVE      0x0C
#define MFRC522_CMD_MF_AUTHENT      0x0E
#define MFRC522_CMD_SOFT_RESET      0x0F
/* MFRC522 Registers */
#define MFRC522_REG_COMMAND         0x01
#define MFRC522_REG_COM_IRQ         0x04
#define MFRC522_REG_DIV_IRQ         0x05
#define MFRC522_REG_ERROR           0x06
#define MFRC522_REG_STATUS1         0x07
#define MFRC522_REG_STATUS2         0x08
#define MFRC522_REG_FIFO_DATA       0x09
#define MFRC522_REG_FIFO_LEVEL      0x0A
#define MFRC522_REG_CONTROL         0x0C
#define MFRC522_REG_BIT_FRAMING     0x0D
#define MFRC522_REG_MODE            0x11
#define MFRC522_REG_TX_CONTROL      0x14
#define MFRC522_REG_TX_ASK          0x15
#define MFRC522_REG_CRC_RESULT_H    0x21
#define MFRC522_REG_CRC_RESULT_L    0x22
#define MFRC522_REG_MOD_WIDTH       0x24
#define MFRC522_REG_T_MODE          0x2A
#define MFRC522_REG_T_PRESCALER     0x2B
#define MFRC522_REG_T_RELOAD_H      0x2C
#define MFRC522_REG_T_RELOAD_L      0x2D
#define MFRC522_REG_VERSION         0x37
/* PICC Commands (Card) */
#define PICC_CMD_REQA               0x26
#define PICC_CMD_WUPA               0x52
#define PICC_CMD_SEL_CL1            0x93
#define PICC_CMD_HLTA               0x50
/* Status Codes */
#define MFRC522_STATUS_OK           0
#define MFRC522_STATUS_ERROR        1
#define MFRC522_STATUS_COLLISION    2
#define MFRC522_STATUS_TIMEOUT      3
#define MFRC522_STATUS_NO_ROOM      4
#define MFRC522_STATUS_INTERNAL_ERROR 5

#define RFID_DEBOUNCE_TIME_MS       2000

typedef struct {
    uint8_t size;           // UID size (4, 7, or 10 bytes)
    uint8_t uid_byte[10];
    uint8_t sak;
} rfid_uid_t;

esp_err_t rfid_init(void);

void rfid_deinit(void);

void rfid_write_register(uint8_t reg, uint8_t value);

uint8_t rfid_read_register(uint8_t reg);

void rfid_send_command(uint8_t command);

bool rfid_check_new_card(void);

bool rfid_read_card_uid(char *uid_buffer, size_t buffer_size);

void rfid_halt_card(void);

void rfid_reset(void);

void rfid_antenna_on(void);

void rfid_antenna_off(void);

#endif