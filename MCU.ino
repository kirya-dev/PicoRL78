#define UART_READ_TIMEOUT_US 2000


void MCU_init_uart() {
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    uart_init(uart0, 115200);
}

void MCU_glitch_init() {
    buf[0] = set_sys_clock_khz(250000, false);
    Serial_print_hex("Overclock: ", buf, 1);

    gpio_init(MCU_GLITCH_PIN);
    gpio_set_dir(MCU_GLITCH_PIN, GPIO_OUT);
    gpio_set_slew_rate(MCU_GLITCH_PIN, GPIO_SLEW_RATE_FAST);
    gpio_set_drive_strength(MCU_GLITCH_PIN, GPIO_DRIVE_STRENGTH_12MA);
}

int64_t alert_MCU_glitch(alarm_id_t id, void *user_data) {
    gpio_put(MCU_GLITCH_PIN, 1);
    sleep_us(random(6.3));
    gpio_put(MCU_GLITCH_PIN, 0);

    cancel_alarm(id);
    return 0;
}

void MCU_break_condition() {
    gpio_init(MCU_RST_PIN);
    gpio_set_dir(MCU_RST_PIN, GPIO_OUT);

    gpio_put(MCU_RST_PIN, 0);
    uart_set_break(uart0, 1);
    sleep_ms(50);
    gpio_put(MCU_RST_PIN, 1);
    sleep_ms(5);
    uart_set_break(uart0, 0);
    sleep_ms(1);

    MCU_uart_clear();
}

void MCU_send_buf(size_t buf_len) {
    uart_write_blocking(uart0, buf, buf_len);
    MCU_read_nonblocking(buf_len);  // сразу из очереди RX очищаем все, так как RXTX - TOOL0, nonblocking потому что проц может что-то выдавать и нарушится буфер.
}

void MCU_send_cmd(size_t cmd_len) {
    size_t i = 0;
    buf[i++] = 1;
    buf[i++] = cmd_len;
    i += cmd_len;
    buf[i++] = cmd_checksum(&buf[1], cmd_len+1);
    buf[i++] = 3;
    MCU_send_buf(i);

    MCU_read_cmd();
}

void MCU_read_cmd() {
    memset(buf, 0xA5, sizeof(buf));
    MCU_read_blocking(2);
    size_t len = buf[1] + 2;
    uart_read_blocking(uart0, &buf[2], len);
    // Serial_print_hex("MCU resp: ", buf, len + 2);
}

void MCU_read_blocking(size_t len) {
    uart_read_blocking(uart0, buf, len);
}

void MCU_read_nonblocking(size_t len) {
    memset(buf, 0xA5, len);
    size_t i = 0, timer = micros();
    while ((i < len) && (micros() - timer < UART_READ_TIMEOUT_US)) {
        if (uart_is_readable(uart0)) {
            buf[i++] = uart_get_hw(uart0)->dr;
        } else {
            tight_loop_contents();
        }
    }
}

void MCU_uart_clear() {
    while (uart_is_readable(uart0)) uart_get_hw(uart0)->dr;
}

uint8_t cmd_checksum(uint8_t *dst, size_t len) {
    size_t sum = 0;
    while (len--) {
        sum += dst[len];
    }

    return 0x100 - sum;
}
