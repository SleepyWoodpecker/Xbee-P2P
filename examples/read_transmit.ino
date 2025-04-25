#include "Xbee.h"

#include <Arduino.h>
#include <HardwareSerial.h>

#define RECEIVE_RESPONSE_TIMEOUT 5000

#define TX 17
#define RX 16
#define RTS 14
#define CTS 15

Xbee xbee(TX, RX, RTS, CTS, 0);


void setup() {
    Serial.begin(9600);
    delay(1000);

    xbee.enter_API_mode();

    Serial.println("\nIts over");
}

void loop() {
    uint8_t response_buffer[128];
    xbee.read_Tx_API_frame(response_buffer, 128);

    Serial.printf("Received: %s\n\n", response_buffer);
}
