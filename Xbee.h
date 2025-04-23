#ifndef XBEE_H_
#define XBEE_H_

#include <cstdint>
#include <Arduino.h>

class Xbee {
    public:
        /*
        * Here, the rts and cts refer to the pins that are connected to the 
        */
        Xbee(uint8_t tx, uint8_t rx, uint8_t rts, uint8_t cts, int baud_rate = 9600);

        /*
        * Get the 64-bit hardware address of the Xbee. 
        *
        * @param address_string: the buffer in which to read the 64-bit address into. Buffer length should be no smaller than 65.
        * 
        * @return true if the hardware address was read successfully, false otherwise
        */
        bool get_hardware_address(char* address_string);
    private:
        uint8_t _tx;  // data out to Xbee
        uint8_t _rx;  // data in from Xbee
        uint8_t _cts; // clear to send
        uint8_t _rts; // ready to send

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

        /*
        * Read the response returned by the Xbee through the rx line
        *
        * @param response_buffer: the buffer to read the response into
        * @param response_buffer_length: the length of the response buffer
        * @param response_buffer_idx&: the buffer index which will be modified, passed by reference
        * 
        * @return true if the read was successful, false otherwise
        */
        bool _read_response(char* response_buffer, size_t response_buffer_length, size_t& response_buffer_idx);

        /*
        * Constructs an AT command, based on the following syntax:
        *   - AT<CMD><param>\r
        *
        * @param response_buffer: the buffer to put the AT command in
        * @param response_buffer_length: the size of the response buffer
        * @param command: the ASCII command that should be sent to the module
        * @param param: the param that you wish to pass in for that command, -1 if there is no param to be passed
        */
        void _construct_AT_command(char* response_buffer, size_t response_buffer_length, const char* command, const int param);
};

#endif