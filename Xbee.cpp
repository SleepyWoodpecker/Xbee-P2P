#include "Xbee.h"

// NOTE: note sure if this is a problem that this becomes hardware dependent, but we will rock with it for now
#include <Arduino.h>
#include <string.h>

// use another serial port for Serial communication with the Xbee
#define XBEE_SERIAL Serial1

#define ENTER_COMMAND_STRING "+++"
#define COMMAND_STRING_ACK_LENGTH 4
#define COMMAND_STRING_ACK "OK\r"

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

bool Xbee::_enter_command_mode() {
    delay(1100);
    _send_command(ENTER_COMMAND_STRING);
    delay(1100);

    char ack_message[COMMAND_STRING_ACK_LENGTH];
    size_t ack_message_idx = 0;

    while (XBEE_SERIAL.available()) {
        ack_message[ack_message_idx++] = (char)XBEE_SERIAL.read();
    }

    return strcmp(ack_message, COMMAND_STRING_ACK) == 0;
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