#include "mbed.h"
#include "ds3231.h"
#include <chrono>
#include "getdistance.h"
#include "motor.h"

#include "SDCard.h"

DigitalOut buzzer(A0);

//Motor Wheel(D13,D11,D9,D10);      //Instance of the Motor Class called 'Wheel' see motor.h and motor.cpp

//Variable 'duty' for programmer to use to vary speed as required set here to #define compiler constant see above
float duty = 0.9;

// Set up the serial port 
static UnbufferedSerial serial_port(D1, D0);  // RX, TX pins

// Define row pins (outputs)
DigitalOut row1(D5);
DigitalOut row2(D6);
DigitalOut row3(D7);
DigitalOut row4(D4);

// Define column pins (inputs with pull-up resistors)
DigitalIn col1(A4, PullUp);
DigitalIn col2(A5, PullUp);
DigitalIn col3(A2, PullUp); 
DigitalIn col4(A3, PullUp);


bool lastKeyState[4][4] = {false};  // For 4 rows x 4 columns keypad

// Define correct password (example: "1234")
char correctPassword[] = {'1', '2', '3', '4'};
char enteredPassword[4];  // Array to store entered password
int passwordIndex = 0;    // Index to track entered password position

void scanKeypad() {
    // Set all rows to HIGH initially
    row1 = 1;
    row2 = 1;
    row3 = 1;
    row4 = 1;

    // Scan rows one by one
    for (int row = 0; row < 4; row++) {
        // Activate the current row by setting it LOW
        if (row == 0) row1 = 0;
        if (row == 1) row2 = 0;
        if (row == 2) row3 = 0;
        if (row == 3) row4 = 0;

        // Check each column for key press or release
        for (int col = 0; col < 4; col++) {
            bool currentKeyState = (col == 0 ? col1 : (col == 1 ? col2 : (col == 2 ? col3 : col4))).read() == 0;  // Detect key press (LOW)
            
            // If the key is released (column goes HIGH) and the key was previously pressed, record the key
            if (currentKeyState && !lastKeyState[row][col]) {
                char key = '\0';  // Initialize the key variable

                // Map the row and column to the corresponding key
                if (row == 0) {
                    if (col == 0) key = '1';
                    if (col == 1) key = '2';
                    if (col == 2) key = '3';
                    if (col == 3) key = 'A';
                }
                else if (row == 1) {
                    if (col == 0) key = '4';
                    if (col == 1) key = '5';
                    if (col == 2) key = '6';
                    if (col == 3) key = 'B';
                }
                else if (row == 2) {
                    if (col == 0) key = '7';
                    if (col == 1) key = '8';
                    if (col == 2) key = '9';
                    if (col == 3) key = 'C';
                }
                else if (row == 3) {
                    if (col == 0) key = '*';
                    if (col == 1) key = '0';
                    if (col == 2) key = '#';
                    if (col == 3) key = 'D';
                }

                // If the key is valid, add it to the entered password
                if (key != '\0' && passwordIndex < 4) {
                    enteredPassword[passwordIndex++] = key;  // Store the entered key
                    printf("Key %c entered\n", key);  // Print the entered key


                    // Activate the buzzer for 1 second when a valid key is pressed
                    buzzer = 1;  // Turn on buzzer
                    wait_us(250000);  // Wait for 1 second
                    buzzer = 0;  // Turn off buzzer
                }

                

                // If the password is fully entered, check it
                if (passwordIndex == 4) {
                    // Compare entered password with correct password
                    bool correct = true;
                    for (int i = 0; i < 4; i++) {
                        if (enteredPassword[i] != correctPassword[i]) {
                            correct = false;
                            break;
                        }
                    }

                    // Print the result based on the comparison
                    if (correct) {
                        printf("Password correct!\n");
                    } else {
                        printf("Incorrect password!\n");
                    }

                    // Reset the password entry
                    passwordIndex = 0;
                }
            }
            // Update the last key state to the current state
            lastKeyState[row][col] = currentKeyState;

        }

        // Deactivate the current row by setting it HIGH
        if (row == 0) row1 = 1;
        if (row == 1) row2 = 1;
        if (row == 2) row3 = 1;
        if (row == 3) row4 = 1;
    }
}

//AnalogIn sensorPin(A0);  // Analog input pin connected to Pressure Sensor output

// Define buffer size to store the RFID tag bytes
uint8_t tag_data[12];  
int Curr_Byte = 0;
int ID = 0;

Ds3231 rtc(D14, D15); // Create RTC object

// Create structures to hold the date and time data
ds3231_time_t Time;
ds3231_calendar_t Calendar; 

// SPI pins (example for NUCLEO-F429ZI)
SPI spi(PB_15, PB_14, PB_13);  // MOSI, MISO, SCK
DigitalOut cs(PC_10);           // Chip Select

SDCard sd(D11, D12, D13, D10, NC);

int main() {
    

    int err = sd.print_file("RFID_Tags.txt", 0); 



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

    while(true) {

        // Wheel.Speed(duty,duty);
        // wait_us(500000);
        // Wheel.Speed(0,0);
        // wait_us(1000000);

        //scanKeypad();
        //wait_us(100000); // Debounce delay

        // float sensorValue = sensorPin.read();  // Read the analog value (0.0 to 1.0)
        // printf("Sensor Value: %.3f\n", sensorValue);
        // wait_us(500000);  // Wait for half a second before reading again

        // int dist = getdistance()/10;
        // printf("distance is %dcm\n", dist);
        // wait_us(100000);


        if (serial_port.readable()) {
            // Read one byte from the RFID reader
            int Byte = serial_port.read(&tag_data[Curr_Byte], 1);

            // Check if the byte is valid
            if (Byte > 0) {
                Curr_Byte++;  // Move to the next byte in the buffer
                
                // Check if we've received the full tag
                if (Curr_Byte >= 12) {
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
                    Curr_Byte = 0;
                }
            }
        }
    }
}