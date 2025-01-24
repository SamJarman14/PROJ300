#include "uop_msb.h"
#include "mbed.h"
#include "ID12RFID.h"
#include "ds3231.h"
#include <chrono>

// Hello World for printing RFID tag numbers

// Set up the serial port (adjust pins as needed)
static UnbufferedSerial serial_port(D1, D0);  // RX, TX pins

// Define buffer size to store the RFID tag bytes
uint8_t tag_data[12];  // Assuming a max of 12 bytes (adjust based on your RFID tag format)
int tag_index = 0;
int ID = 0;

I2C i2c(D14, D15); // SDA, SCL pins
Ds3231 rtc(D14, D15); // Create RTC object

// Create a structure to hold the time data
ds3231_time_t Time;
int main() {

    // Get the current time from the RTC
        rtc.get_time(&Time);

        // Print the time
        printf("Time: %02d:%02d:%02d\n", Time.hours, Time.minutes, Time.seconds);

    while(1) {
        if (serial_port.readable()) {
            // Read one byte from the RFID reader
            int num_bytes = serial_port.read(&tag_data[tag_index], 1);

            // Check if the byte is valid
            if (num_bytes > 0) {
                tag_index++;  // Move to the next byte in the buffer
                
                // Check if we've received the full tag
                // (For example, if the tag is 12 bytes long, adjust as necessary)
                if (tag_index >= 12) {
                    // Tag data received, print the full tag
                    //printf("RFID Tag received: ");
                    // for (int i = 0; i < 12; i++) {
                    //     printf("%02X \n", tag_data[i]);
                    // }
                    //printf("\n");

                    // Convert the first 4 bytes into the first int
                    int Tag1 = (tag_data[0] << 24) | (tag_data[1] << 16) | (tag_data[2] << 8) | tag_data[3];
                    
                    // Convert the next 4 bytes into the second int
                    int Tag2 = (tag_data[4] << 24) | (tag_data[5] << 16) | (tag_data[6] << 8) | tag_data[7];

                    // Convert the last 4 bytes into the third int
                    int Tag3 = (tag_data[8] << 24) | (tag_data[9] << 16) | (tag_data[10] << 8) | tag_data[11];

                    if ((Tag1 == 0x02334130) && (Tag2 == 0x30364245) && (Tag3== 0x37464334)){
                        ID = 1;
                    }
                    else if ((Tag1 == 0x02334130) && (Tag2 == 0x30364333) && (Tag3== 0x31464239)){
                        ID = 2;
                    }
                    else if ((Tag1 == 0x02303530) && (Tag2 == 0x30323036) && (Tag3== 0x46334337)){
                        ID = 3;
                    }
                    else if ((Tag1 == 0x02303530) && (Tag2 == 0x30323037) && (Tag3== 0x35374432)){
                        ID = 4;
                    }

                    if (ID >= 1 && ID <= 4){
                        printf("Tag ID = %d\n", ID);
                    }
                    else {
                        printf("Invalid ID Tag\n");
                    }
                    

                    // // Print the integers in hexadecimal
                    // printf("Tag Part One: %08X\n", Tag1);
                    // printf("Tag Part Two: %08X\n", Tag2);
                    // printf("Tag Part Three: %08X\n", Tag3);

                    // Reset the tag buffer for the next tag
                    tag_index = 0;
                }
            }
        }
    }
}