#include "Xbee.h"

#include <Arduino.h>
#include <HardwareSerial.h>

#define TX 17
#define RX 16
#define RTS 14
#define CTS 15

void setup() {
    Serial.begin(9600);
    delay(1000);

    Xbee xbee(TX, RX, RTS, CTS);

    xbee.enter_API_mode();

    xbee.get_upper_bits_of_hardware_address_api();

    uint8_t bs[] = {0x7E, 0x00, 0x14, 0x10, 0x01, 0x00, 0x13, 0xA2, 0x00, 0x42, 0x4A, 0x12, 0xCD, 0xFF, 0xFE, 0x00, 0x00, 0x57, 0x41, 0x53, 0x53, 0x55, 0x50, 0xEE};
    for (int i = 0; i < 24; ++i) {
        Serial2.write(bs[i]);
    }

    Serial.println("\nIts over");
}

void loop() {

}