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

    Serial.println("\nIts over");
}

void loop() {

}
