#include "Keypad.h"
#include "RFID.h"

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

DigitalOut buzzer(A1);  // Buzzer

// Define correct password 
char correctPassword[] = {'1', '2', '3', '4'};
char enteredPassword[4];  // Array to store entered password
int passwordIndex = 0;    // Index to track entered password position
bool lastKeyState[4][4] = {false};  // 4 rows x 4 columns keypad

// Track incorrect attempts and timeout flag
int incorrectAttempts = 0;
bool timeoutFlag = false;

// Function to reset the timeout and incorrect attempts
void resetTimeout() {
    incorrectAttempts = 0;
    timeoutFlag = true;
    buzzer = 1;  // Alert with buzzer
    wait_us(2000000);     // Buzzer alert for 1 second
    buzzer = 0;
    wait_us(10000000);    // 10 seconds timeout
    timeoutFlag = false;
}

void scanKeypad() {
    // If in timeout, skip the scanning process
    if (timeoutFlag) {
        return;
    }

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

            // Detect key press (LOW)
            bool currentKeyState;
            if (col == 0) {
                currentKeyState = (col1.read() == 0);
            } else if (col == 1) {
                currentKeyState = (col2.read() == 0);
            } else if (col == 2) {
                currentKeyState = (col3.read() == 0);
            } else {
                currentKeyState = (col4.read() == 0);
            }
            
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
                    buzzer = 1;  
                    wait_us(250000);  
                    buzzer = 0; 
                }

                // If the password is fully entered, check if correct
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
                    
                        incorrectAttempts++;  // Increment incorrect attempt counter

                        // If 3 incorrect attempts, trigger alert and timeout
                        if (incorrectAttempts >= 3) {
                            esp32.write("T\n", 2); // Send the signal to the ESP32 to trigger email alert to phone
                            printf("\nToo many incorrect attempts. Access locked for 10 seconds.\n");
                            resetTimeout();
                        }
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