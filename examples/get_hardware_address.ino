#include "Xbee.h"

#include <Arduino.h>
#include <HardwareSerial.h>

#define TX 17
#define RX 16
#define RTS 14
#define CTS 15

void setup() {
    Xbee xbee(TX, RX, RTS, CTS, 0);

    Serial.begin(9600);
    delay(1000);

    Serial.println("Trying it out...");

    xbee.enter_API_mode();

    Serial.println("In API mode");
    
    xbee.get_upper_bits_of_hardware_address_api();
}

void loop() {

} 