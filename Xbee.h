#ifndef XBEE_H_
#define XBEE_H_

#include <cstdint>
#include <Arduino.h>

enum operation_modes {
    TRANSPARENT,
    API
};

class Xbee {
    public:
        /*
        * Here, the rts and cts refer to the pins that are connected to the 
        */
        Xbee(uint8_t tx, uint8_t rx, uint8_t rts, uint8_t cts, int baud_rate = 9600, operation_modes op_mode);


    private:
        uint8_t _tx;  // data out to Xbee
        uint8_t _rx;  // data in from Xbee
        uint8_t _cts; // clear to send
        uint8_t _rts; // ready to send

        operation_modes _operation_mode;

        /*
        * Tries to enter command mode on the Xbee, by sending in '+++'.
        * 
        * There should at least be 1 second preceding and following the '+++' sequence.
        * 
        * @return true if command mode was successfully entered ("OK\r" received). false otherwise
        */
        bool _enter_command_mode();

        /*
        * Check that the module is ready for receiving data before sending it out
        *
        * @param cmd_string: the command that you wish to send to the Xbee
        * @param max_retry_count: the number of times you want to retry sending the data to the Xbee if the Xbee's input buffer is full
        * 
        * @return true if the command was sent successfully
        */
        bool _send_command(char* cmd_string, int max_retry_count = 3);
};

#endif