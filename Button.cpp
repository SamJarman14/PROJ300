#include "Button.h"
#include "RFID.h"
#include "Keypad.h"
    
volatile bool buttonPressed = false;    // Flag set in ISR
Timer debounceTimer;
Timer gate_timer;                       // Timer to track open gate time

// Button interrupt handler
void onButtonPress() {
    if (debounceTimer.elapsed_time().count() >= 200000) {       // Check elapsed time before flagging (debounce)
        buttonPressed = true;
        debounceTimer.reset(); 
    }
}

void monitor_button(){    
    if (buttonPressed) {
        buttonPressed = false;          // Clear flag
        uart_mutex.lock();
        esp32.write("G\n", 2);          // Send signal to ESP32 to open the pedestrian gate
        uart_mutex.unlock();

        char complete = 0;

        // Wait for 'C' confirmation after the motor action
        while (!esp32.readable()) {
            ThisThread::sleep_for(50ms);
        }

        esp32.read(&complete, 1);

        if (complete == 'C') {
            printf("Complete\n");
        } else {
            printf("ERROR: Unknown Serial Communication Command: %c\n", complete);
        }

        string Exit_Entry = "Exit";
        char buffer[64];            // Buffer to hold data
        string passcode_name = "Unknown";

        // Format the data into the buffer
        sprintf(buffer, "\n\n%s\n%s\n%02d/%02d/20%02d %02d:%02d:%02d\n",
            Exit_Entry.c_str(), passcode_name.c_str(), Calendar.date, Calendar.month, Calendar.year,
            Time.hours, Time.minutes, Time.seconds);

        // Write the formatted string to the SD card
        sd.write_file("log.txt", buffer, true, false);

        wait_us(250000);                    // Wait 0.25s for limit switch to be released after gate opens

        gate_timer.start();                 // Start the timer when the gate is opened
        bool gateOpened = false;            // Tracks if the gate was opened
        bool warningPrinted = false;        // Flag to track if warning was printed

        while (true) {
            // Check if gate is open (limit switch not pressed)
            if (gateSwitch == 0) {          // Gate is OPEN when the limit switch is HIGH (not pressed)
                if (!gateOpened) {
                    gateOpened = true;      // Mark that the gate was opened
                    printf("Gate opened.\n");
                }

                // Check if the gate has been open for 10 seconds
                if (gate_timer.elapsed_time() >= 10s && !warningPrinted) {
                    uart_mutex.lock();
                    esp32.write("T\n", 2);          // Send the signal to the ESP32 to trigger email alert to phone
                    uart_mutex.unlock();
                    printf("Warning: Gate left open!\n");
                    warningPrinted = true;          // Set flag so it doesn't print again
                }
            } else if (gateSwitch == 1) {           // Gate is closed (limit switch pressed)
                if (gateOpened) {
                    // If the gate was previously opened, trigger closing signal
                    printf("Closing gate...\n");
                    uart_mutex.lock();
                    esp32.write("C\n", 2);          // Send signal to ESP32 to close the pedestrian gate
                    uart_mutex.unlock();
                    gateOpened = false;             // Reset the flag
                }

                gate_timer.stop();          // Stop the timer when the gate is closed
                gate_timer.reset();         // Reset the timer when gate is closed
                warningPrinted = false;     // Reset flag for next time
                break;                      // Exit the loop once the gate is closed
            }
        }
    }
}