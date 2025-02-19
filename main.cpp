#include "mbed.h"
#include "ds3231.h"
#include <chrono>
#include "getdistance.h"
#include "motor.h"

#include <sstream>
#include <unordered_set>


#include "SDCard.h"

DigitalOut buzzer(A0);

Motor Wheel(D13,D11,D9,D10);      //Instance of the Motor Class called 'Wheel' see motor.h and motor.cpp

//Variable 'duty' for programmer to use to vary speed as required 
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
uint8_t tag_data[12], add_data[12];  
int Curr_Byte = 0, curr_Byte = 0;
int ID = 0;

Ds3231 rtc(D14, D15); // Create RTC object

// Create structures to hold the date and time data
ds3231_time_t Time;
ds3231_calendar_t Calendar; 

// SPI pins (example for NUCLEO-F429ZI)
SPI spi(PB_15, PB_14, PB_13);  // MOSI, MISO, SCK
DigitalOut cs(PC_10);           // Chip Select

SDCard sd(D11, D12, D13, D10, NC);

// Vector to store the numbers as strings
std::vector<std::string> rfid_tags;  // Stores ordered list of tags
std::unordered_set<std::string> rfid_set;  // For quick lookups

void load_tags_from_sd() {
    std::vector<std::string> buffer;

    if (sd.read_file("RFID_Tags.txt", buffer, false) == 0) {  
        rfid_tags.clear();  // Clear old tags
        rfid_set.clear();  

        for (const std::string& line : buffer) {
            if (!line.empty()) {
                size_t space_pos = line.find(' ');  
                std::string tag_id = line.substr(0, 24);  // Extract tag ID
                
                // Remove spaces
                std::string clean_tag(tag_id);
                clean_tag.erase(std::remove_if(clean_tag.begin(), clean_tag.end(), ::isspace), clean_tag.end());

                rfid_tags.push_back(line);  // Store full tag (ID + Name)
                rfid_set.insert(clean_tag);  // Store only the cleaned tag ID for quick lookup
            }
        }

        // Print the loaded RFID tags
        printf("\nActive Tags Loaded from SD Card:\n");
        for (size_t i = 0; i < rfid_tags.size(); ++i) {
            printf("Tag %zu: %s\n", i + 1, rfid_tags[i].c_str());
        }
    } else {
        printf("No existing RFID_Tags.txt found or failed to read file.\n");
    }
}

void process_tag(bool isAdding) {
    printf(isAdding ? "\nAdd function selected\nPlease scan the RFID tag...\n" : "\nRemove function selected\nPlease scan the RFID tag...\n");

    char tag_string[25];
    curr_Byte = 0;
    Timer timer;
    timer.start();

    while (true) {
        if (timer.elapsed_time().count() > 10000000) {
            printf("\nTag scan timed out\nReturning to menu...\n");
            return;
        }

        if (serial_port.readable()) {
            int Byte = serial_port.read(&add_data[curr_Byte], 1);
            if (Byte > 0) {
                curr_Byte++;

                if (curr_Byte >= 12) {
                    for (int i = 0; i < 12; ++i) {
                        sprintf(tag_string + i * 2, "%02X", add_data[i]);
                    }
                    tag_string[24] = '\0';

                    // Master tag should NOT be added
                    const char master_tag[] = "023341303036424537464334";
                    if (isAdding && strcmp(tag_string, master_tag) == 0) {
                        printf("Error: Master tag cannot be added as a regular tag.\n");
                        return;
                    }

                   
                    if (isAdding) {
                        std::string clean_tag(tag_string);
                        clean_tag.erase(std::remove_if(clean_tag.begin(), clean_tag.end(), ::isspace), clean_tag.end());

                        if (rfid_set.find(clean_tag) != rfid_set.end()) {
                            printf("Error: Tag already exists.\n");
                            return;
                        }

                        char tag_name[50] = "";
                        printf("Enter a name for this tag: ");
                        scanf(" %49[^\n]", tag_name);
                        while (getchar() != '\n' && getchar() != EOF);


                        std::string tag_entry = clean_tag + " " + tag_name;
                        rfid_tags.push_back(tag_entry);
                        rfid_set.insert(clean_tag);

                        printf("\nTag added successfully: %s (%s)\n", tag_string, tag_name);

                        std::string all_tags;
                        for (const auto &tag : rfid_tags) {
                            all_tags += tag + "\n";
                        }

                        std::vector<char> buffer(all_tags.begin(), all_tags.end());
                        buffer.push_back('\0');

                        sd.write_file("RFID_Tags.txt", buffer.data(), false, false);

                    }   
                    else {
                        auto it = std::find_if(rfid_tags.begin(), rfid_tags.end(),
                        [&](const std::string &entry) {
                            return entry.substr(0, 24) == tag_string;
                        });

                        if (it != rfid_tags.end()) {
                            std::string tag_name = "Unknown";
                            size_t space_pos = it->find(' ');
                            if (space_pos != std::string::npos) {
                                tag_name = it->substr(space_pos + 1);
                            }

                            rfid_tags.erase(it);
                            rfid_set.erase(tag_string);  // Remove from set as well
                            printf("\nTag removed successfully: %s (%s)\n", tag_string, tag_name.c_str());

                            std::string all_tags;
                            for (const auto &tag : rfid_tags) {
                                all_tags += tag + "\n";
                            }

                            std::vector<char> buffer(all_tags.begin(), all_tags.end());
                            buffer.push_back('\0');

                            sd.write_file("RFID_Tags.txt", buffer.data(), false, false);
                        } else {
                            printf("Tag not found.\n");
                        }
                    }

                    // Update SD card file
                    std::string all_tags;
                    for (const auto& tag : rfid_tags) {
                        all_tags += tag + "\n";
                    }

                    // Copy to mutable char array
                    std::vector<char> buffer(all_tags.begin(), all_tags.end());
                    buffer.push_back('\0');  // Null-terminate


                    sd.write_file("RFID_Tags.txt", buffer.data(), false, false);
                    return;
                }
            }
        }
    }
}

void rename_tag() {
    printf("\nRename function selected\nPlease scan the RFID tag to rename...\n");

    char tag_string[25];
    curr_Byte = 0;
    Timer timer;
    timer.start();

    while (true) {
        if (timer.elapsed_time().count() > 10000000) {
            printf("\nTag scan timed out\nReturning to menu...\n");
            return;
        }

        if (serial_port.readable()) {
            int Byte = serial_port.read(&add_data[curr_Byte], 1);
            if (Byte > 0) {
                curr_Byte++;

                if (curr_Byte >= 12) {
                    for (int i = 0; i < 12; ++i) {
                        sprintf(tag_string + i * 2, "%02X", add_data[i]);
                    }
                    tag_string[24] = '\0';

                    // Search for the tag in the system
                    auto it = std::find_if(rfid_tags.begin(), rfid_tags.end(),
                        [&](const std::string& entry) {
                            return entry.substr(0, 24) == tag_string;
                        });

                    if (it != rfid_tags.end()) {
                        // Extract the current name
                        std::string old_name = "Unknown";
                        size_t space_pos = it->find(' ');
                        if (space_pos != std::string::npos) {
                            old_name = it->substr(space_pos + 1);
                        }

                        // Prompt user for a new name
                        char new_name[50];
                        printf("Current Name: %s\nEnter new name: ", old_name.c_str());
                        scanf(" %49[^\n]", new_name);  // Read new name

                        // Flush input buffer (Only ONCE after scanf)
                        while (getchar() != '\n' && getchar() != EOF);

                        // Update the tag's name
                        *it = std::string(tag_string) + " " + new_name;
                        printf("Tag renamed successfully: %s -> %s\n", old_name.c_str(), new_name);

                        // Update SD card file
                        std::string all_tags;
                        for (const auto& tag : rfid_tags) {
                            all_tags += tag + "\n";
                        }

                        std::vector<char> buffer(all_tags.begin(), all_tags.end());
                        buffer.push_back('\0');

                        sd.write_file("RFID_Tags.txt", buffer.data(), false, false);

                    } else {
                        printf("Tag not found in the system.\n");
                    }
                    
                    return;  // Ensures function exits after processing
                }
            }
        }
    }
}


void master_function()
{
    char user_input[8];    // space for characters and pressing enter
    while(true){
        printf("\n\nEnter master access password...\nOr enter 'exit' to quit\n");
        // Use fgets instead of scanf to safely read input
        fgets(user_input, sizeof(user_input), stdin);
        // Remove trailing newline from fgets input
        user_input[strcspn(user_input, "\n")] = '\0';

        if (strcmp(user_input, "jarman") == 0) {
            printf("\n\nMaster Card Access Granted");
            while(true){

                printf("\nPlease choose a function...\nType 'add' to add a new RFID tag to the system\nType 'remove' to remove a RFID tag from the system\nType 'rename' to rename a tag\nType 'exit' to exit master access\n");

                // Use fgets instead of scanf to safely read input
                fgets(user_input, sizeof(user_input), stdin);
                // Remove trailing newline from fgets input
                user_input[strcspn(user_input, "\n")] = '\0';

                if (strcmp(user_input, "add") == 0) {
                    process_tag(true);
                }
                else if (strcmp(user_input, "remove") == 0) {
                   process_tag(false);
                }
                else if (strcmp(user_input, "rename") == 0) {
                    rename_tag();  // Call the rename function
                } 
                else if (strcmp(user_input, "exit") == 0) {
                    // Handle exit case
                    printf("Master access exited\n");
                    return;  // Exit function
                } 
                else {
                    // If the input doesn't match any of the above
                    printf("Invalid input\n");
                }
            }
        }
        else if(strcmp(user_input, "exit") == 0) {
            // Handle exit case
            printf("Master access exited\n");
            return;  // Exit function
        }
        else{
            // If the input doesn't match any of the above
            printf("Incorrect Password\n");
        }
    }   
}


int main() {
    
    // Load tags into memory from SD card at startup
    load_tags_from_sd();

    while(true) {

        // Wheel.Speed(duty,duty);
        // wait_us(500000);
        // Wheel.Speed(0,0);
        // wait_us(1000000);

        //scanKeypad();
        //wait_us(100000); // Debounce delay

        // int dist = getdistance()/10;
        // printf("distance is %dcm\n", dist);
        // wait_us(100000);


        if (serial_port.readable()) {
            // Read one byte from the RFID reader
            int Byte = serial_port.read(&tag_data[Curr_Byte], 1);

            // To skip checking for a match when master card is scanned
            bool skip_match_check = false;

            // Check if the byte is valid
            if (Byte > 0) {
                Curr_Byte++;  // Move to the next byte in the buffer
                
                // Check if we've received the full tag
                if (Curr_Byte >= 12) {

                    // Create a string large enough to hold the hexadecimal representation
                    char tag_string[25] = {0};  // 12 bytes * 2 hex digits per byte + null terminator
                    
                    // Convert tag_data[] to a hexadecimal string
                    for (int i = 0; i < 12; ++i) {
                        // Convert each byte to two hexadecimal digits and append to tag_string
                        sprintf(tag_string + i * 2, "%02X", tag_data[i]);
                        
                    }
                    tag_string[24] = '\0'; // Ensure null termination

                    char master[] = "023341303036424537464334";

                    if (strcmp(tag_string, master) == 0) {
                        master_function();
                        skip_match_check = true;
                    }

                    bool match = false;

                    // Check if scanned tag exists in the unordered_set for fast lookup
                    if (rfid_set.find(tag_string) != rfid_set.end()) {
                        match = true;
                    }

                    if ((match == true) && (skip_match_check == false)){
                        printf("Tag match found: %s\n", tag_string);

                        //Get the current time from the RTC
                        rtc.get_time(&Time);
                        // Get the current date from the RTC
                        rtc.get_calendar(&Calendar);
                        
                        // Print the time and date
                        printf("%02d/%02d/20%02d %02d:%02d:%02d\n", Calendar.date, Calendar.month, Calendar.year, Time.hours, Time.minutes, Time.seconds);
                    }
                    else if((match == false) && (skip_match_check == false)){
                        printf("No matching tag found.\n");
                    }
                    else{
                        skip_match_check = false;
                    }

                    // Reset the tag buffer for the next tag
                    Curr_Byte = 0;
                }
            }
        }
    }
}