#include "Xbee.h"

#include <Arduino.h>
#include <HardwareSerial.h>

#define RECEIVE_RESPONSE_TIMEOUT 5000

#define TX 17
#define RX 16
#define RTS 14
#define CTS 15

void setup() {
    Serial.begin(9600);
    delay(1000);

    Xbee xbee(TX, RX, RTS, CTS);

    xbee.enter_API_mode();

    // xbee.get_upper_bits_of_hardware_address_api();
    char address_string[100];
    xbee.get_hardware_address(address_string);
    Serial.println(address_string);

    delay(1000);

    Serial.print("GEttig ready to send\n\n");

    uint8_t bs[] = {0x7E, 0x00, 0x10, 0x10, 0x01, 0x00, 0x13, 0xA2, 0x00, 0x42, 0x4A, 0x12, 0xCD, 0xFF, 0xFE, 0x00, 0x00, 0x48, 0x49, 0x40};
    for (int i = 0; i < 20; ++i) {
        Serial2.write(bs[i]);
    }

    Serial.println("\nIts over");
}

void loop() {
    byte byteSequence[] = {
    0x7E, 0x00, 0x14, 0x10, 0x01, 0x00, 0x13, 0xA2, 
    0x00, 0x42, 0x4F, 0xAA, 0x3E, 0xFF, 0xFE, 0x00, 
    0x00, 0x53, 0x48, 0x45, 0x45, 0x53, 0x48, 0x03
  };
  
  // Send the byte sequence
  Serial2.write(byteSequence, sizeof(byteSequence));
  Serial.println("Packet Sent!");
  
  delay(1000); // Wait for a second before sending again
}

bool _read_byte_response(uint8_t* response_buffer, size_t response_buffer_length, size_t& response_buffer_idx) {
    unsigned long start_time = millis();
    while ((millis() - start_time) < RECEIVE_RESPONSE_TIMEOUT) {
        if (!Serial2.available()) {
            continue;
        }
        unsigned char input = Serial2.read();
        response_buffer_idx++;
        Serial.printf("%x ", input);
    }
    return response_buffer_idx > 0;
}