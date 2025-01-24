#include "ID12RFID.h"

// Constructor - initializes the serial port with the specified rx pin
ID12RFID::ID12RFID(PinName rx) : _rfid(rx, NC, 9600) {  // UnbufferedSerial(rx, baudrate)
    // 9600 baud rate is the default for ID12LA
}

// Function to check if the RFID reader has sent data (i.e., tag ID)
int ID12RFID::readable() {
    return _rfid.readable(); // Returns non-zero value if data is available
}

// Function to read the RFID tag ID (blocking)
int ID12RFID::read() {
    int tag = 0;
    uint8_t byte;  // Temporary byte variable to hold each byte

    while (!_rfid.readable()) { // Wait until data is available
        // Wait for a tag to be scanned
    }

    // Read the tag data (assuming the ID is a 4-byte integer)
    _rfid.read(&byte, 1);  // Read 1 byte
    tag = byte;  // Store the first byte in the tag

    _rfid.read(&byte, 1);  // Read 1 more byte
    tag = (tag << 8) | byte;  // Shift and add the second byte

    _rfid.read(&byte, 1);  // Read 1 more byte
    tag = (tag << 8) | byte;  // Shift and add the third byte

    _rfid.read(&byte, 1);  // Read 1 more byte
    tag = (tag << 8) | byte;  // Shift and add the fourth byte

    return tag; // Return the full tag ID
}
