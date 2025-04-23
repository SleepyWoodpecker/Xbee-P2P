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

    Xbee xbee(RX, RX, RTS, CTS);
    
    char hardware_address[65];
    xbee.get_hardware_address(hardware_address);

    Serial.print("Hardware address is: ");
    Serial.println(hardware_address);
}

void loop() {

}