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

DigitalOut buzzer(A1);              // Buzzer

DigitalIn gateSwitch(D8, PullUp);   // Limit switch 

Timer gateTimer;                    // Timer to track open gate time

Mutex uart_mutex;                   // Mutex for protecting UART communication

// Define variables
char enteredPasscode[7];            // Array to store entered password
int passcodeIndex = 0;              // Index to track entered password position
bool lastKeyState[4][4] = {false};  // 4 rows x 4 columns keypad
bool correct;

// Track incorrect attempts and timeout flag
int incorrectAttempts = 0;
bool timeoutFlag = false;

int Attempt_Count = 0;                      // Counter for incorrect password attempts
bool Lockout_Message_Printed = false;       // Flag to prevent multiple lockout messages
bool Access_Locked = false;                 // Flag for master access lockout
time_t Lockout_Start_Time = 0;              // Time when the lockout started

string Entry_Exit = "Unknown";

unordered_set<string> passcode_set;         // Set to store the passcodes
vector<string> passcode_entries;            // Vector to store the passcode and name


// Function to reset the timeout and incorrect attempts
void resetTimeout() {
    incorrectAttempts = 0;
    timeoutFlag = true;
    buzzer = 1;             // Alert with buzzer
    wait_us(2000000);       // Buzzer alert for 1 second
    buzzer = 0;
    wait_us(10000000);      // 10 seconds timeout
    timeoutFlag = false;
}

void emergency_vehicle_access(){
    // Send signal to ESP32 to open the vehicle gate
    uart_mutex.lock();
    esp32.write("O\n", 2); 
    uart_mutex.unlock();

    char entry = 0;
    string Entry_Exit = "Unknown";

    // Wait until ESP32 sends entry status ('Y' or 'N')
    while (!esp32.readable()) {
        ThisThread::sleep_for(50ms);        // Wait a bit before checking again
    }

    esp32.read(&entry, 1);        // Read feedback sent from ESP32

    if (entry == 'Y') {           // If 'Y' is received, it was an entry event 
        printf("Entry\n");
        Entry_Exit = "Entry";
    } 
    else if (entry == 'N') {      // If 'N' is received, it was an exit event 
        printf("Exit\n");
        Entry_Exit = "Exit";
    } 
    else {
        printf("ERROR: Unknown Serial Communication Command: %c\n", entry);
    }

    char complete = 0;

    // Wait for 'C' confirmation after the motor action
    while (!esp32.readable()) {
        ThisThread::sleep_for(50ms);
    }

    esp32.read(&complete, 1);

    if (complete == 'C') {                  
        printf("Complete\n");
    } 
    else {
        printf("ERROR: Unknown Serial Communication Command: %c\n", complete);
    }

    char buffer[64];            // Buffer to hold data
    char passcode_name[50] = "";

    // Format the data into the buffer
    sprintf(buffer, "\n\n%s\n%s\n%02d/%02d/20%02d %02d:%02d:%02d\n",
        Entry_Exit.c_str(), passcode_name, Calendar.date, Calendar.month, Calendar.year,
        Time.hours, Time.minutes, Time.seconds);

    // Write the formatted string to the SD card
    sd.write_file("log.txt", buffer, true, false);
}

void load_passcodes_from_sd() {
    vector<string> buffer;

    if (sd.read_file("passcodes.txt", buffer, false) == 0) {  
        passcode_set.clear();           // Clear old passcodes
        passcode_entries.clear(); 

        for (const string& line : buffer) {
            if (!line.empty()) {
                size_t space_pos = line.find(' ');          // Find space between passcode and name
                string passcode = line.substr(0, 6);        // Extract just passcode
                
                // Remove spaces in passcode
                string clean_passcode(passcode);
                clean_passcode.erase(remove_if(clean_passcode.begin(), clean_passcode.end(), ::isspace), clean_passcode.end());

                passcode_entries.push_back(line);               // Store passcode and name
                passcode_set.insert(clean_passcode);            // Store only the cleaned passcode for quick lookup
            }
        }

        // Print the loaded passcodes
        printf("\nActive Passcodes Loaded from SD Card:\n");
        for (size_t i = 0; i < passcode_entries.size(); ++i) {
            printf("Passcode %zu: %s\n", i + 1, passcode_entries[i].c_str());
        }
    } else {
        printf("No existing passcodes.txt found or failed to read file.\n");
    }
}

void process_passcode(bool isAdding) {
    char user_input[8];             // Space for characters and pressing enter

    if (isAdding) {
    printf("\nAdd function selected\nPlease enter the new passcode...\n");
    } 
    else {
        load_passcodes_from_sd();
        printf("\nRemove function selected\nPlease enter the passcode from the list above to be removed...\n");
    }

    while (true) {

        // Use fgets to read user input
        fgets(user_input, sizeof(user_input), stdin);
        // Remove trailing newline from input
        user_input[strcspn(user_input, "\n")] = '\0';

        // Master passcode should can not be added
        const char master_passcode[] = "714147";
        if (isAdding && strcmp(user_input, master_passcode) == 0) {
            printf("Error: Master passcode cannot be added as a regular passcode.\n");
            return;
        }

        // For adding a passcode
        if (isAdding) {

            // Clean the user input - remove any spaces
            string clean_passcode(user_input);
            clean_passcode.erase(remove_if(clean_passcode.begin(), clean_passcode.end(), ::isspace), clean_passcode.end());

            // Can't add a passcode that's already in the system (prevents duplicates)
            if (passcode_set.find(clean_passcode) != passcode_set.end()) {
                printf("Error: Passcode already exists.\n");
                return;
            }

            // Prompt user to enter a name for the passcode
            char passcode_name[50] = "";
            printf("Enter a name for this passcode: ");
            fgets(passcode_name, sizeof(passcode_name), stdin);

            // Remove the newline character at the end if present
            size_t len = strlen(passcode_name);
            if (len > 0 && passcode_name[len - 1] == '\n') {
                passcode_name[len - 1] = '\0';
            }

            // Combine the passcode with it's associated name
            string passcode_entry = clean_passcode + " " + passcode_name;
            // Add to list of stored passcodes
            passcode_entries.push_back(passcode_entry);
            passcode_set.insert(clean_passcode);

            printf("\nPasscode added successfully: %s (%s)\n", clean_passcode.c_str(), passcode_name);

            // Prepare all passcodes for writing to the SD card (adding new lines)
            string all_passcodes;
            for (const auto &passcode : passcode_entries) {
                all_passcodes += passcode + "\n";
            }

            // Convert string to a character buffer
            vector<char> buffer(all_passcodes.begin(), all_passcodes.end());
            buffer.push_back('\0');

            // Write all passcodes to SD card
            sd.write_file("passcodes.txt", buffer.data(), false, false);
        }   
        
        // For when removing a passcode
        else {
            // Searches the first 6 digits on each line for a matching passcode (ignores the name)
            auto PASSCODE = find_if(passcode_entries.begin(), passcode_entries.end(),
            [&](const string &entry) {
                return entry.substr(0, 6) == user_input;
            });

            // If a matching passcode is found
            if (PASSCODE != passcode_entries.end()) {
                string passcode_name = "Unknown";           // default for if the passcode doesn't have a name
                size_t space_pos = PASSCODE->find(' ');
                // Extract the name of the found passcode
                if (space_pos != string::npos) {
                    passcode_name = PASSCODE->substr(space_pos + 1);
                }

                // Remove passcode from all lists 
                passcode_entries.erase(PASSCODE);
                passcode_set.erase(user_input);  
                printf("\nPasscode removed successfully: %s (%s)\n", user_input, passcode_name.c_str());

                // Rebuild the passcode list 
                string all_passcodes;
                for (const auto &passcode : passcode_entries) {
                    all_passcodes += passcode + "\n";
                }

                // Prepare list for writing to SD card
                vector<char> buffer(all_passcodes.begin(), all_passcodes.end());
                buffer.push_back('\0');

                // Write updated passcode list to SD card
                sd.write_file("passcodes.txt", buffer.data(), false, false);
            } else {
                printf("Passcode not found.\n");
            }
        }
        return;
    }
}

void rename_passcode() {
    char user_input[8];    // space for characters and pressing enter

    printf("\nRename function selected\nPlease enter the passcode to rename...\n");

    while (true) {

        // Read passcode to be renamed
        // Use fgets to read user input
        fgets(user_input, sizeof(user_input), stdin);
        // Remove trailing newline from input
        user_input[strcspn(user_input, "\n")] = '\0';

        // Search for the passcode in the system
        auto PASSCODE = find_if(passcode_entries.begin(), passcode_entries.end(),
            [&](const string& entry) {
                return entry.substr(0, 6) == user_input;
            });

        if (PASSCODE != passcode_entries.end()) {
            // Extract the current name
            string old_name = "Unknown";
            size_t space_pos = PASSCODE->find(' ');
            if (space_pos != string::npos) {
                old_name = PASSCODE->substr(space_pos + 1);
            }

            // Prompt user for a new name
            char new_name[50];
            printf("Current Name: %s\nEnter new name: ", old_name.c_str());
            fgets(new_name, sizeof(new_name), stdin);           // Read new name

            // Remove the newline character at the end if present
            size_t len = strlen(new_name);
            if (len > 0 && new_name[len - 1] == '\n') {
                new_name[len - 1] = '\0';
            }

            // Update the passcode's name
            *PASSCODE = string(user_input) + " " + new_name;
            printf("Passcode renamed successfully: %s -> %s\n", old_name.c_str(), new_name);

            // Update SD card file
            string all_passcodes;
            for (const auto& passcode : passcode_entries) {
                all_passcodes += passcode + "\n";
            }

            vector<char> buffer(all_passcodes.begin(), all_passcodes.end());
            buffer.push_back('\0');

            sd.write_file("passcodes.txt", buffer.data(), false, false);

        } else {
            // If invalid passcode is scanned
            printf("Passcode not found in the system.\n");
        }
        
        return;  // Ensures function exits after processing
    }
}


void passcode_master_function() {
    char user_input[8];             // space for characters and pressing enter

    while(true) {
        // Check if there was a timeout since last incorrect attempt
        if (Attempt_Count >= 3) {
            time_t current_time = time(NULL);
            double seconds_elapsed = difftime(current_time, Lockout_Start_Time);

            if (seconds_elapsed < 10) {
                // If less than 30 seconds have passed since the last failed attempt, deny access
                if (!Lockout_Message_Printed) {
                    printf("\nAccess Locked. Please wait before trying again.\n");
                    Access_Locked = true;               // Lock access
                    Lockout_Message_Printed = true;     // Flag the message as printed
                }
                // Skip the password check, just continue the loop until timeout
                return;             // Recheck the timeout condition in the next loop iteration
            } else {
                // Reset the counter after timeout
                Attempt_Count = 0;
                Lockout_Message_Printed = false;        // Reset the flag after the lockout period
                Access_Locked = false;                  // Unlock access
            }
        }

        // Only ask for password if not locked out
        printf("\n\nEnter master access password...\nOr enter 'exit' to quit\n");

        // Use fgets to read user input
        fgets(user_input, sizeof(user_input), stdin);
        // Remove trailing newline from input
        user_input[strcspn(user_input, "\n")] = '\0';

        // If user enters 'exit'
        if (strcmp(user_input, "exit") == 0) {
            printf("\nMaster access exited\n");
            return;                             // Exit function
        }

        // If user enters correct password 
        if (strcmp(user_input, "samwj") == 0) {
            printf("\n\nMaster Access Granted");
            while(true){
                // Provide user with functions to add, remove and rename passcodes or to exit master access
                printf("\nPlease choose a function...\nType 'add' to add a new Passcode to the system\nType 'remove' to remove a Passcode from the system\nType 'rename' to rename a passcode\nType 'exit' to exit master access\n");

                // Use fgets to read user input
                fgets(user_input, sizeof(user_input), stdin);
                // Remove trailing newline from fgets input
                user_input[strcspn(user_input, "\n")] = '\0';

                if (strcmp(user_input, "add") == 0) {
                    process_passcode(true);
                }
                else if (strcmp(user_input, "remove") == 0) {
                    process_passcode(false);
                }
                else if (strcmp(user_input, "rename") == 0) {
                    rename_passcode();              // Call the rename function
                } 
                else if (strcmp(user_input, "exit") == 0) {
                    printf("\nMaster access exited\n");
                    return;             // Exit function
                } 
                else {
                    // If the input doesn't match any of the above
                    printf("\nInvalid input\n");
                }
            }
        }
        else {
            // If the input doesn't match any of the above
            Attempt_Count++;                    // Increment the incorrect password attempt count
            Lockout_Start_Time = time(NULL);    // Record the time of the incorrect attempt
            printf("\nIncorrect Password\n");
            
            if (Attempt_Count >= 3) {
                // Action after 3 incorrect attempts
                uart_mutex.lock();
                esp32.write("T\n", 2);          // Send the signal to the ESP32 to trigger email alert to phone
                uart_mutex.unlock();
                printf("\nToo many incorrect attempts. Access locked for 10 seconds.\n");
                Access_Locked = true;           // Lock access
                return;
            }
        }
    }
}


void scanKeypad() {
    while(true) {
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
                    char key = '\0';            // Initialize the key variable

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
                    if (key != '\0' && passcodeIndex < 6) {
                        enteredPasscode[passcodeIndex++] = key;         // Store the entered key
                        printf("Key %c entered\n", key);                // Print the entered key

                        // Activate the buzzer for 1 second when a valid key is pressed
                        buzzer = 1;  
                        wait_us(250000);  
                        buzzer = 0; 
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

        // Once password is fully entered, check if it's correct
        if (passcodeIndex == 6) {
            // Null-terminate the entered passcode string
            enteredPasscode[passcodeIndex] = '\0';

            // Create a string from the entered passcode
            string entered_passcode_str(enteredPasscode, 6);

            // To skip checking for a match when master passcode is entered
            bool skip_match_check = false;

            char master[] = "714147";               // Master access passcode for comparison
            char vehicle_backup[] = "ABCABC";       // Emergency vehicle gate access passcode for comparison
            
            if (strcmp(entered_passcode_str.c_str(), master) == 0) {
                skip_match_check = true;            // Set flag to skip checking the passcode (master passcode isn't in the system)
                passcode_master_function();         // Proceed with master access    
            }
            else if (strcmp(entered_passcode_str.c_str(), vehicle_backup) == 0) {
                skip_match_check = true;            // Set flag to skip checking the passcode (emergency vehicle access passcode isn't in the system)
                emergency_vehicle_access();         // Proceed with emergency vehicle access access    
            }

            bool match = false;  

            // Check if scanned passcode exists in the system - unordered_set for fast lookup
            if (passcode_set.find(entered_passcode_str) != passcode_set.end()) {
                match = true;
            }

            // Check if the entered passcode exists in the passcode_set
            if ((match == true) && (skip_match_check == false)) {
                string passcode_name = "Unknown";

                // Iterate through all stored passcodes to find a match           
                for (const auto& entry : passcode_entries) {
                    if (entry.substr(0, 6) == entered_passcode_str) {   // Compare first 6 characters (passcode) with user input
                        size_t space_pos = entry.find(' ');             // Find position of first space (separates ID from name)
                        // If a space is found, extract the passcode name 
                        if (space_pos != string::npos) {
                            passcode_name = entry.substr(space_pos + 1);  // Assign name
                        }
                        break;                      // Exit the loop early since the matching passcode has been found
                    }
                }

                entered_passcode_str.erase(remove_if(entered_passcode_str.begin(), entered_passcode_str.end(), ::isspace), entered_passcode_str.end());
                passcode_name.erase(remove_if(passcode_name.begin(), passcode_name.end(), ::isspace), passcode_name.end());

                // Print the passcode and name
                printf("\nPasscode match found: %s (%s)\n", entered_passcode_str.c_str(), passcode_name.c_str());

                // Get the current time from the RTC
                rtc.get_time(&Time);
                // Get the current date from the RTC
                rtc.get_calendar(&Calendar);
                
                // Print the time and date the passcode was scanned
                printf("%02d/%02d/20%02d %02d:%02d:%02d\n", Calendar.date, Calendar.month, Calendar.year, Time.hours, Time.minutes, Time.seconds);

                passcodeIndex = 0;              // Reset after entry 

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
                } 
                else {
                    printf("ERROR: Unknown Serial Communication Command: %c\n", complete);
                }

                string Exit_Entry = "Entry";

                char buffer[64];            // Buffer to hold data

                // Format the data into the buffer
                sprintf(buffer, "\n\n%s\n%s\n%02d/%02d/20%02d %02d:%02d:%02d\n",
                    Exit_Entry.c_str(), passcode_name.c_str(), Calendar.date, Calendar.month, Calendar.year,
                    Time.hours, Time.minutes, Time.seconds);

                // Write the formatted string to the SD card
                sd.write_file("log.txt", buffer, true, false);

                wait_us(250000);                // Wait 0.25s for limit switch to be released after gate opens 

                gateTimer.start();              // Start the timer when the gate is opened
                bool gateOpened = false;        // Tracks if the gate was opened
                bool warningPrinted = false;    // Flag to track if warning was printed

                while (true) {
                    // Check if gate is open (limit switch not pressed)
                    if (gateSwitch == 0) {      // Gate is OPEN when the limit switch is HIGH (not pressed)
                        if (!gateOpened) {
                            gateOpened = true;  // Mark that the gate was opened
                            printf("Gate opened.\n");
                        }

                        // Check if the gate has been open for 10 seconds
                        if (gateTimer.elapsed_time() >= 10s && !warningPrinted) {
                            uart_mutex.lock();
                            esp32.write("L\n", 2);          // Send the signal to the ESP32 to trigger email alert to phone
                            uart_mutex.unlock();
                            printf("Warning: Gate left open!\n");
                            warningPrinted = true;          // Set flag so it doesn't print again
                        }
                    } 
                    else if (gateSwitch == 1){              // Gate is closed (limit switch pressed)
                        if (gateOpened) {  
                            // If the gate was previously opened, trigger closing signal
                            printf("Closing gate...\n");
                            uart_mutex.lock();
                            esp32.write("C\n", 2);          // Send signal to ESP32 to close the pedestrian gate
                            uart_mutex.unlock();
                            gateOpened = false;             // Reset the flag
                        }

                        gateTimer.stop();               // Stop the timer when the gate is closed
                        gateTimer.reset();              // Reset the timer when gate is closed
                        warningPrinted = false;         // Reset flag for next time
                        break;                          // Exit the loop once the gate is closed
                    }

                    // Add a small delay to prevent the loop from running too fast
                    ThisThread::sleep_for(100ms);
                }
            }
            else if ((match == false) && (skip_match_check == false)) {
                printf("Incorrect password!\n");
                incorrectAttempts++;            // Increment incorrect attempt counter

                if (incorrectAttempts >= 3) {
                    uart_mutex.lock();
                    esp32.write("I\n", 2);      // Trigger alert for too many incorrect attempts
                    uart_mutex.unlock();
                    printf("\nToo many incorrect attempts. Access locked for 10 seconds.\n");
                    resetTimeout();
                }
            }
            else {
                // Reset the flag to skip check if master passcode is entered
                skip_match_check = false;
            }

            passcodeIndex = 0;          // Reset after entry 
        }

        ThisThread::sleep_for(100ms);  // Avoid overloading the CPU
    }
}



