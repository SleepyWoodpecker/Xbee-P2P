#ifndef XBEE_H_
#define XBEE_H_

#include <cstdint>
#include <Arduino.h>

class Xbee {
    public:
        /*
        * Here, the rts and cts refer to the pins that are connected to the 
        */
        Xbee(uint8_t tx, uint8_t rx, uint8_t rts, uint8_t cts, uint64_t destination_address, int baud_rate = 9600);

        /*
        * Get the 64-bit hardware address of the Xbee. 
        *
        * @param address_string: the buffer in which to read the 64-bit address into. Buffer length should be no smaller than 65.
        * 
        * @return true if the hardware address was read successfully, false otherwise
        */
        bool get_hardware_address(char* address_string);

        /*
        * Get the maximum number of bytes that can be sent per RF transmission
        *
        * @return the number of bytes that can be sent per RF transmission
        */
        int get_max_bytes_per_req();

        /*
        * Get the upper 32 bits of the XBee's 64-bit address using the API frame. Mostly for testing purposes
        *
        * @return the upper 32 bits of the address as a decimal
        */
        int get_upper_bits_of_hardware_address_api();

        /*
        * Enter API mode 1 (AP = 1)
        * 
        * @returns true if the transition succeeds, false otherwise
        */
       bool enter_API_mode();

       size_t read_Tx_API_frame(uint8_t* response_buffer, size_t response_buffer_size);

    private:
        uint8_t _tx;  // data out to Xbee
        uint8_t _rx;  // data in from Xbee
        uint8_t _cts; // clear to send
        uint8_t _rts; // ready to send

        uint64_t _destination_address;

        /*
        * Tries to enter command mode on the Xbee, by sending in '+++'.
        * 
        * There should at least be 1 second preceding and following the '+++' sequence.
        * 
        * @return true if command mode was successfully entered ("OK\r" received). false otherwise
        */
        bool _enter_command_mode();

        /*
        * Exit API mode
        *
        * @return true if the exit was successful
        */
        bool _exit_command_mode();

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
        * Formats Xbee commands as a byte string, required API mode
        *
        * @param cmd_string: the API frame to send over serial
        * @param length: the length of the API frame
        * @param: the maximum number of retries that the command string will make if the receive buffer is full
        * 
        * @return true if the byte command was sent successfully, false otherwise
        */
        bool _send_byte_command(uint8_t* cmd_string, size_t length, int max_retry_count = 3);

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
        * Read the byte response from the Xbee in API Mode. Should start with 0x7E
        *
        * @param response_buffer: the buffer to read the response into
        * @param response_buffer_length: the length of the response buffer
        * @param response_buffer_idx: the index of the response buffer to start writing to. passed by reference
        * 
        * @return true if some bytes are read
        */
        bool _read_byte_response(uint8_t* response_buffer, size_t response_buffer_length, size_t& response_buffer_idx, size_t message_offset_from_start);

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

        /*
        * Constructs an API frame to create a local AT request
        *
        * @param payload: the byte string payload that will be put into the frame
        * 
        * @return the size of the api frame created
        */
        size_t _construct_AT_API_frame(uint8_t* api_frame_buffer, size_t api_frame_buffer_length, char* payload, uint8_t api_command, char* at_command, uint8_t frame_number = 1);

        /*
        * Constructs an API frame to create a Tx request
        *
        * @param api_frame_buffer: the buffer to write the request frame into
        * @param api_frame_buffer_length: the size of the frame buffer
        * @param payload: the byte string that will be put into the frame // NOTE: need to test what happens if the byte string > 100 bytes
        * @param frame_number: the frame number of the API frame
        * 
        * @return the size of the API frame created
        */
        size_t _construct_Tx_API_frame(uint8_t* api_frame_buffer, size_t api_frame_buffer_length, char* payload, uint8_t frame_number = 1);

};

#endif