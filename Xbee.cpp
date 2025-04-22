#include "Xbee.h"

// NOTE: note sure if this is a problem that this becomes hardware dependent, but we will rock with it for now
#include <Arduino.h>
#include <string.h>

// use another serial port for Serial communication with the Xbee
#define XBEE_SERIAL Serial1

#define ENTER_COMMAND_STRING "+++"
#define COMMAND_STRING_ACK_LENGTH 4
#define COMMAND_STRING_ACK "OK\r"
#define COMMAND_STRING_DELAY 1000

#define AT_COMMAND_STRING_LENGTH 10
#define RECEIVE_STRING_MAX_LENGTH 64

Xbee::Xbee(uint8_t tx, uint8_t rx, uint8_t rts, uint8_t cts, int baud_rate = 9600, operation_modes op_mode)
    :_tx(tx), _rx(rx), _rts(rts), _cts(cts), _operation_mode(op_mode)
{
    // SERIAL_8N1 tells the serial port dictates that each frame is 8 bits with 1 stop bit
    XBEE_SERIAL.begin(baud_rate, SERIAL_8N1, _rx, _tx);

    // setup the ready to send and clear to send pins
    // data should only be sent when the microcontroller is ready to send and the module is clear to send
    pinMode(_rts, OUTPUT);
    pinMode(_cts, INPUT);

    // rts pin indicates that the microcontroller is ready to read the 
    // data from the module, and it is active low
    digitalWrite(_rts, LOW);
}

bool Xbee::get_hardware_address(char* address_string) {
    char upper_32_address[RECEIVE_STRING_MAX_LENGTH];
    size_t upper_32_address_idx = 0;
    char lower_32_address[RECEIVE_STRING_MAX_LENGTH];
    size_t lower_32_address_idx = 0;

    char command_string[AT_COMMAND_STRING_LENGTH];
    
    // read the first 32 bits
    _construct_AT_command(command_string, AT_COMMAND_STRING_LENGTH, "SH", 0);
    
    if (!_send_command(command_string)) {
        Serial.println("Failed to send command to get hardware address");
        return 0;
    }

    // add timeout to give module time to respond to the command string
    unsigned long start_time = millis();
    bool read_from_serial = false;

    while ((millis() - start_time) < COMMAND_STRING_DELAY && !read_from_serial) {
        read_from_serial = true;

        while (XBEE_SERIAL.available() && upper_32_address_idx < RECEIVE_STRING_MAX_LENGTH- 1) {
            char input = XBEE_SERIAL.read();
            upper_32_address[upper_32_address_idx++] = input;

            // might be dangerous that I am not checking for any kind of stop byte here?
        }
    } 

    upper_32_address[upper_32_address_idx++] = '\0';
    // expect a string of length 33, since the output is 32 bits plus \0 at end
    if (upper_32_address_idx < 32) {
        Serial.println("Did not receive the right number of address bytes");
        return false;
    }

    // reset values to read from SL
    start_time = millis();
    read_from_serial = false;

    while ((millis() - start_time) < COMMAND_STRING_DELAY && !read_from_serial) {
        read_from_serial = true;

        while (XBEE_SERIAL.available() && lower_32_address_idx < RECEIVE_STRING_MAX_LENGTH- 1) {
            char input = XBEE_SERIAL.read();
            lower_32_address[lower_32_address_idx++] = input;

            // might be dangerous that I am not checking for any kind of stop byte here?
        }
    } 

    lower_32_address[lower_32_address_idx++] = '\0';
    // expect a string of length 33, since the output is 32 bits plus \0 at end
    if (lower_32_address_idx < 32) {
        Serial.println("Did not receive the right number of address bytes");
        return false;
    }

    sprintf(address_string, "%s%s", upper_32_address, lower_32_address);

    return true;
}

void Xbee::_construct_AT_command(char* response_buffer, size_t response_buffer_length, const char* command, const int param) {
    snprintf(response_buffer, response_buffer_length, "AT%s%d\r", command, param);
}

bool Xbee::_enter_command_mode() {
    delay(1100);
    _send_command(ENTER_COMMAND_STRING);
    delay(1100);

    char ack_message[COMMAND_STRING_ACK_LENGTH];
    size_t ack_message_idx = 0;

    // add timeout to give module time to respond to the command string
    unsigned long start_time = millis();
    bool read_from_serial = false;

    while ((millis() - start_time) < COMMAND_STRING_DELAY && !read_from_serial) {
        read_from_serial = true;

        while (XBEE_SERIAL.available() && ack_message_idx < COMMAND_STRING_ACK_LENGTH - 1) {
            char input = XBEE_SERIAL.read();
            ack_message[ack_message_idx++] = input;
            
            if (input == '\r') {
                break;
            }
        }
    } 
    ack_message[ack_message_idx++] = '\0';

    if (strcmp(ack_message, COMMAND_STRING_ACK) != 0) {
        Serial.print("Received invalid response: ");
        Serial.println(ack_message);
        return false;
    }

    return true;
}

bool Xbee::_send_command(char* cmd_string, int max_retry_count = 3) {
    int try_count = 0;

    // check that the module is ready to receive, which is indicated by the CTS pin being low
    while (digitalRead(_cts) == HIGH && try_count < max_retry_count) {
        ++try_count;
        delay(10);
    }

    XBEE_SERIAL.print(cmd_string);
    
    return try_count < max_retry_count;
}