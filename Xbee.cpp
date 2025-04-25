#include "Xbee.h"
#include "AT_Commands.h"
#include "API_Frame.h"

// NOTE: note sure if this is a problem that this becomes hardware dependent, but we will rock with it for now
#include <Arduino.h>
#include <string.h>
#include <stdlib.h>

// use another serial port for Serial communication with the Xbee
#define XBEE_SERIAL Serial2

#define COMMAND_STRING_ACK_LENGTH 4
#define RECEIVE_RESPONSE_TIMEOUT 5000

#define AT_COMMAND_STRING_LENGTH 10
#define RECEIVE_STRING_MAX_LENGTH 64
#define API_FRAME_MAX_LENGTH 128

Xbee::Xbee(uint8_t tx, uint8_t rx, uint8_t rts, uint8_t cts, uint64_t destination_address, int baud_rate)
    :_tx(tx), _rx(rx), _rts(rts), _cts(cts), _destination_address(destination_address)
{
    // SERIAL_8N1 tells the serial port dictates that each frame is 8 bits with 1 stop bit
    XBEE_SERIAL.begin(baud_rate, SERIAL_8N1, _rx, _tx);

    // // setup the ready to send and clear to send pins
    // // data should only be sent when the microcontroller is ready to send and the module is clear to send
    // pinMode(_rts, OUTPUT);
    // pinMode(_cts, INPUT);

    // // rts pin indicates that the microcontroller is ready to read the 
    // // data from the module, and it is active low
    // digitalWrite(_rts, LOW);
    // delay(1500);
    // Serial.print("+++");
    // delay(1500);

}

bool Xbee::get_hardware_address(char* address_string) {
    if (!_enter_command_mode()) {
        Serial.println("Unable to enter command mode");
        return false;
    }

    char command_string[AT_COMMAND_STRING_LENGTH];
    
    // read the first 32 bits
    _construct_AT_command(command_string, AT_COMMAND_STRING_LENGTH, SERIAL_ADDRESS_HIGH, -1);
    if (!_send_command(command_string)) {
        Serial.println("Failed to send command to get hardware address");
        return 0;
    }

    char upper_32_address[RECEIVE_STRING_MAX_LENGTH];
    size_t upper_32_address_idx = 0;
    if (!_read_response(upper_32_address, RECEIVE_STRING_MAX_LENGTH, upper_32_address_idx)) {
        Serial.println(upper_32_address);
        Serial.println("Did not receive the right number of address bytes for upper address");
        return false;
    }

    // read the next 32 bits
    _construct_AT_command(command_string, AT_COMMAND_STRING_LENGTH, SERIAL_ADDRESS_LOW, -1);
    if (!_send_command(command_string)) {
        Serial.println("Failed to send command to get hardware address");
        return 0;
    }

    char lower_32_address[RECEIVE_STRING_MAX_LENGTH];
    size_t lower_32_address_idx = 0;
    if (!_read_response(lower_32_address, RECEIVE_STRING_MAX_LENGTH, lower_32_address_idx)) {
        Serial.println("Did not receive the right number of address bytes for lower address");
        return false;
    }

    sprintf(address_string, "%s%s", upper_32_address, lower_32_address);

    return true;
}

int Xbee::get_max_bytes_per_req() {
    if (!_enter_command_mode()) {
        Serial.println("Unable to enter command mode");
        return -1;
    }

    char command_string[AT_COMMAND_STRING_LENGTH];
    _construct_AT_command(command_string, AT_COMMAND_STRING_LENGTH, MAXIMUM_PACKET_PAYLOAD_BYTES, -1);
    
    char response_buffer[RECEIVE_STRING_MAX_LENGTH];
    size_t response_buffer_idx = 0;
    if (!_send_command(command_string)) {
        Serial.println("Failed to send command to get max number of bytes per RF request");
        return -1;
    }

    if (!_read_response(response_buffer, RECEIVE_STRING_MAX_LENGTH, response_buffer_idx)) {
        Serial.println("Failed to read response for max number of bytes per RF request");
        return -1;
    }

    return atoi(response_buffer);
}

int Xbee::get_upper_bits_of_hardware_address_api() {
    uint8_t api_frame[API_FRAME_MAX_LENGTH];
    char at_command[] = SERIAL_ADDRESS_HIGH;

    size_t length = _construct_AT_API_frame(api_frame, API_FRAME_MAX_LENGTH, "", API_FRAME_LOCAL_AT_COMMAND_REQUEST, at_command);

    if (!_send_byte_command(api_frame, length)) {
        Serial.println("Failed to send command to get max number of bytes per RF request");
        return -1;
    }

    size_t response_buffer_idx = 0;
    if (!_read_byte_response(api_frame, API_FRAME_MAX_LENGTH, response_buffer_idx, API_RESPONSE_OFFSET_HARDWARE_ADDRESS)) {
        Serial.println("Error encountered when trying to read byte response");
        return -1;
    }
    
    Serial.println("Writing result");
    for (int i = 0; i < response_buffer_idx - 1; ++i) {
        Serial.printf("%x ", api_frame[i]);
    }

    return -1;
}

bool Xbee::enter_API_mode() {
    if (!_enter_command_mode()) {
        return false;
    }

    char command_string[AT_COMMAND_STRING_LENGTH];
    _construct_AT_command(command_string, AT_COMMAND_STRING_LENGTH, TOGGLE_API_MODE, 1);
    if (!_send_command(command_string)) {
        Serial.println("Failed to write to set AP mode to 1");
        return false;
    }

    char ack_message[COMMAND_STRING_ACK_LENGTH];
    size_t ack_message_idx = 0;
    if (!_read_response(ack_message, COMMAND_STRING_ACK_LENGTH, ack_message_idx) || strcmp(ack_message, COMMAND_MODE_ACK) != 0) {
        Serial.print("Received invalid response when trying to activate command mode: ");
        Serial.printf("%c %zu\n", ack_message[0], strlen(ack_message));
        return false;
    }

    if (!_exit_command_mode()) {
        return false;
    }

    delay(1000);

    return true;
}

size_t Xbee::read_Tx_API_frame(uint8_t* response_buffer, size_t response_buffer_size) {
    size_t response_buffer_idx = 0;
    if (!_read_byte_response(response_buffer, response_buffer_size, response_buffer_idx, API_RESPONSE_RECEIVE_OFFSET)) {
        Serial.println("Invalid response received over radio");
        return 0;
    }

    return response_buffer_idx + 1;
}

void Xbee::_construct_AT_command(char* response_buffer, size_t response_buffer_length, const char* command, const int param) {
    if (param >= 0) {
        snprintf(response_buffer, response_buffer_length, "AT%s%d\r", command, param);
    }
    else {
        snprintf(response_buffer, response_buffer_length, "AT%s\r", command);
    }

    return;
}

size_t Xbee::_construct_AT_API_frame(uint8_t* api_frame_buffer, size_t api_frame_buffer_length, char* payload, uint8_t api_command, char* at_command, uint8_t frame_number) {
    // put in dummy values in the frame so the length can later bec calculated with strlen()
    memset(api_frame_buffer, 0, api_frame_buffer_length); // NOTE: this might not be necessary

    api_frame_buffer[0] = API_FRAME_START_DELIMITER;
    api_frame_buffer[1] = 0x05;
    api_frame_buffer[2] = 0x05;
    api_frame_buffer[3] = API_FRAME_LOCAL_AT_COMMAND_REQUEST;
    api_frame_buffer[4] = frame_number;
    api_frame_buffer[5] = at_command[0];
    api_frame_buffer[6] = at_command[1];

    size_t api_frame_idx = 7; // start putting in the relevant values at index 6
    size_t original_payload_idx = 0;

    // maintain a sum of bytes for checksum calculation later
    uint8_t byte_sum = (
        API_FRAME_LOCAL_AT_COMMAND_REQUEST + 
        frame_number + 
        at_command[0] + // last 2 bits of the at_command
        at_command[1] // first 2 bits of the at_command
    );

    while (
        api_frame_idx < API_FRAME_MAX_LENGTH && 
        payload[original_payload_idx] != '\0'
    ) {
        api_frame_buffer[api_frame_idx] = (unsigned char) payload[original_payload_idx];

        byte_sum += api_frame_buffer[api_frame_idx];
        
        ++original_payload_idx;
        ++api_frame_idx;
    }

    // add in the checksum value
    api_frame_buffer[api_frame_idx] = 0xFF - byte_sum;

    // correct the length value
    uint16_t length = api_frame_idx - 3; // disregard the start delimiter and length
    api_frame_buffer[1] = length >> 8;
    api_frame_buffer[2] = length & 0xFF;

    return api_frame_idx + 1;
}

size_t Xbee::_construct_Tx_API_frame(uint8_t* api_frame_buffer, size_t api_frame_buffer_length, char* payload, uint8_t frame_number) {
    // put in dummy values in the frame so the length can later bec calculated with strlen()
    memset(api_frame_buffer, 0, api_frame_buffer_length); // NOTE: this might not be necessary

    api_frame_buffer[0] = API_FRAME_START_DELIMITER;
    api_frame_buffer[1] = 0x05;
    api_frame_buffer[2] = 0x05;
    api_frame_buffer[3] = API_FRAME_TRANSMIT_REQUEST;
    api_frame_buffer[4] = frame_number;
    
    uint8_t byte_sum = (
        API_FRAME_TRANSMIT_REQUEST +
        frame_number +
        0xFF +
        0xFE +
        0x00 +
        0x00
    );

    // store the destination address in the next 8 bytes
    size_t api_frame_buffer_idx = 5;
    for (int i = 0; i < 8; ++i) {
        uint8_t address_byte = (_destination_address >> (8 * i)) & 0xFF;
        byte_sum += address_byte;
        api_frame_buffer[api_frame_buffer_idx++] = address_byte;
    }

    // set the reserved field
    api_frame_buffer[api_frame_buffer_idx++] = 0xFF;
    api_frame_buffer[api_frame_buffer_idx++] = 0xFE;
    
    // broadcast radius
    api_frame_buffer[api_frame_buffer_idx++] = 0x00;

    // options
    api_frame_buffer[api_frame_buffer_idx++] = 0x00;

    size_t payload_idx = 0;
    while (
        api_frame_buffer_idx < API_FRAME_MAX_LENGTH &&
        payload[payload_idx] != '\0'
    ) {
        api_frame_buffer[api_frame_buffer_idx] = (uint8_t) payload[payload_idx]; 
        byte_sum += api_frame_buffer[api_frame_buffer_idx];
        
        ++api_frame_buffer_idx;
        ++payload_idx;
    }

    // add checksum
    api_frame_buffer[api_frame_buffer_idx] = 0xFF - byte_sum;

    // correct the length
    size_t length = api_frame_buffer_idx - 3;
    api_frame_buffer[1] = (length >> 8) & 0xFF;
    api_frame_buffer[2] = length & 0xFF;

    return api_frame_buffer_idx + 1;
}

bool Xbee::_enter_command_mode() {
    delay(1100);
    _send_command(ENTER_COMMAND_MODE_STRING);
    delay(1100);

    char ack_message[COMMAND_STRING_ACK_LENGTH];
    size_t ack_message_idx = 0;

    if (!_read_response(ack_message, COMMAND_STRING_ACK_LENGTH, ack_message_idx) || strcmp(ack_message, COMMAND_MODE_ACK)!= 0) {
        Serial.print("Received invalid response when trying to activate command mode: ");
        Serial.printf("%c %zu\n", ack_message[0], strlen(ack_message));
        return false;
    }

    return true;
}

bool Xbee::_exit_command_mode() {
    char at_command[AT_COMMAND_STRING_LENGTH];
    _construct_AT_command(at_command, AT_COMMAND_STRING_LENGTH, EXIT_COMMAND_MODE, -1);

    if (!_send_command(at_command)) {
        Serial.println("Failed could not send command to exit command mode");
        return false;
    }

    char ack_message[COMMAND_STRING_ACK_LENGTH];
    size_t ack_message_idx = 0;

    if (!_read_response(ack_message, COMMAND_STRING_ACK_LENGTH, ack_message_idx) || strcmp(ack_message, COMMAND_MODE_ACK)!= 0) {
        Serial.print("Received invalid response when trying to activate command mode: ");
        Serial.printf("%c %zu\n", ack_message[0], strlen(ack_message));
        return false;
    }

    return true;
}

bool Xbee::_send_command(char* cmd_string, int max_retry_count) {
    int try_count = 0;

    // check that the module is ready to receive, which is indicated by the CTS pin being low
    // while (digitalRead(_cts) == HIGH && try_count < max_retry_count) {
    //     ++try_count;
    //     delay(10);
    // }

    size_t wrote = XBEE_SERIAL.write(cmd_string);
    
    return wrote > 0 && try_count < max_retry_count;
}

bool Xbee::_send_byte_command(uint8_t* cmd_string, size_t length, int max_retry_count) {
    int try_count = 0;

    // check that the module is ready to receive, which is indicated by the CTS pin being low
    // while (digitalRead(_cts) == HIGH && try_count < max_retry_count) {
    //     ++try_count;
    //     delay(10);
    // }

    size_t wrote = XBEE_SERIAL.write(cmd_string, length);
    
    return wrote > 0 && try_count < max_retry_count;
}

bool Xbee::_read_response(char* response_buffer, size_t response_buffer_length, size_t& response_buffer_idx) {
    // add timeout to give module time to respond to the command string
    unsigned long start_time = millis();
    bool attempted_read_from_serial = false;

    while ((millis() - start_time) < RECEIVE_RESPONSE_TIMEOUT) {
        if (!XBEE_SERIAL.available()) {
            continue;
        }

        char input = XBEE_SERIAL.read();
        response_buffer[response_buffer_idx++] = input;

        // response is terminated by a '\r'or when the idx of the buffer exceeds its length
        if (input == '\r' || response_buffer_idx >= response_buffer_length - 1) {
            break;
        }
    }

    response_buffer[response_buffer_idx++] = '\0';
    
    return response_buffer_idx > 1;
}

bool Xbee::_read_byte_response(uint8_t* response_buffer, size_t response_buffer_length, size_t& response_buffer_idx, size_t message_offset_from_start) {
    unsigned long start_time = millis();
    bool checksum_valid = false;

    while ((millis() - start_time) < RECEIVE_RESPONSE_TIMEOUT) {
        // wait for the start byte to continue reading
        if (!XBEE_SERIAL.available() || (uint8_t)XBEE_SERIAL.peek() != API_FRAME_START_DELIMITER) {
            continue;
        }

        uint8_t stop_byte = XBEE_SERIAL.read();
        delay(10);
        // the next two bytes will be the length
        uint16_t size_msb = XBEE_SERIAL.read();
        delay(10);
        uint16_t size_lsb = XBEE_SERIAL.read();

        size_t sequence_length = (size_msb << 8) + size_lsb;

        uint8_t byte_sum = 0;
        size_t read = 0;

        unsigned long read_start = millis();
        // then start reading the message
        while (
            read < sequence_length && 
            response_buffer_idx < response_buffer_length && 
            millis() - read_start < RECEIVE_RESPONSE_TIMEOUT
        ) {
            if (!XBEE_SERIAL.available()) {
                continue;
            }


            uint8_t input = XBEE_SERIAL.read();
            ++read;
            byte_sum += input;

            if (read > message_offset_from_start - 3) {
                response_buffer[response_buffer_idx++] = input;
            }
        }
        
        uint8_t xbee_checksum = XBEE_SERIAL.read();
        checksum_valid = xbee_checksum == (0xFF - byte_sum);
        Serial.printf("%x, %x", xbee_checksum, (0xFF - byte_sum));
        break;
    }
    
    if (!checksum_valid) {
        Serial.println("Checksum invalid! Discarding packet");
        memset(response_buffer, 0, response_buffer_length);
        return false;
    }

    // add null byte to the end of the byte sequence so it can be read as a string
    response_buffer[response_buffer_idx++] = '\0';

    return response_buffer_idx > 0;
}   