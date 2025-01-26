#include "uop_msb.h"
#include "mbed.h"
#include "ds3231.h"
#include <chrono>

// Set up the serial port 
static UnbufferedSerial serial_port(D1, D0);  // RX, TX pins

// Define buffer size to store the RFID tag bytes
uint8_t tag_data[12];  
int byteNum = 0;
int ID = 0;

//I2C i2c(D14, D15); // SDA, SCL pins
Ds3231 rtc(D14, D15); // Create RTC object

// Create structures to hold the date and time data
ds3231_time_t Time;
ds3231_calendar_t Calendar; 

int main() {

    // // Set the time
    // time.hours = 15;            // Hour = 12
    // time.minutes = 49;           // Minute = 0
    // time.seconds = 0;           // Second = 0
    // time.am_pm = 0;             // AM (0 for PM, 1 for AM)
    // time.mode = 0;              // 24-hour mode
    // // Set the time on the DS3231
    // uint16_t result = rtc.set_time(time);
    // // Set the date
    // calendar.day = 1;        // Day = Saturday (1=Sunday, ..., 7=Saturday)
    // calendar.date = 26;      // Date = 26
    // calendar.month = 1;      // Month = January
    // calendar.year = 25;      // Year = 25 (representing 2025)
    // // Set the calendar on the DS3231
    // result = rtc.set_calendar(calendar);

    // Get the current time from the RTC
    //rtc.get_time(&Time);
    // Get the current date from the RTC
    //rtc.get_calendar(&Calendar);
    
    // Print the time
    //printf("%02d/%02d/20%02d %02d:%02d:%02d\n", Calendar.date, Calendar.month, Calendar.year, Time.hours, Time.minutes, Time.seconds);
    //printf("Date: %02d/%02d/20%02d\n", calendar.date, calendar.month, calendar.year);

    while(true) {
        if (serial_port.readable()) {
            // Read one byte from the RFID reader
            int num_bytes = serial_port.read(&tag_data[byteNum], 1);

            // Check if the byte is valid
            if (num_bytes > 0) {
                byteNum++;  // Move to the next byte in the buffer
                
                // Check if we've received the full tag
                if (byteNum >= 12) {
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
                        // Get the current time from the RTC
                        rtc.get_time(&Time);
                        // Get the current date from the RTC
                        rtc.get_calendar(&Calendar);
                        
                        // Print the time
                        printf("%02d/%02d/20%02d %02d:%02d:%02d\n", Calendar.date, Calendar.month, Calendar.year, Time.hours, Time.minutes, Time.seconds);

                        // Print the Tag ID
                        printf("Tag ID = %d\n\n", ID);
                    }
                    else {
                        printf("Invalid ID Tag\n\n");
                    }

                    // Reset the tag buffer for the next tag
                    byteNum = 0;
                }
            }
        }
    }
}