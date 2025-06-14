
void Serial_print_hex(const char *comment, uint8_t *dst, size_t len) {
    Serial.print(comment);
    while (len--) {
        if (*dst < 0x10) Serial.print("0");
        Serial.print(*dst++, HEX);
        Serial.print(len ? " " : "\n");
    }
}

void Serial_print_str(const char *comment, uint8_t *dst, size_t len) {
    Serial.print(comment);
    Serial.write(dst, len);
    Serial.println();
}
