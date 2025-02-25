#include "RFID.h"

// Set up the serial port for communication with ESP32
UnbufferedSerial esp32(D1, D0, 115200);  // TX, RX pins
// Set up the serial port for communication with RFID Reader
UnbufferedSerial serial_port(NC, A0);  // TX, RX pins

uint8_t tag_data[12], add_data[12];  // Define buffer size to store the RFID tag bytes
int curr_Byte = 0;

Ds3231 rtc(D14, D15); // Create RTC object
// Create structures to hold the date and time data
ds3231_time_t Time;
ds3231_calendar_t Calendar; 

// SPI pins for DS3231 
SPI spi(PB_15, PB_14, PB_13);  // MOSI, MISO, SCK
DigitalOut cs(PC_10);   // Chip Select

SDCard sd(D11, D12, D13, D10, NC);  // MOSI, MISO, SCLK, CS

// Vector to store the numbers as strings
std::vector<std::string> rfid_tags;  // Stores ordered list of tags
std::unordered_set<std::string> rfid_set;  // For quick lookups

int attempt_count = 0; // Counter for incorrect password attempts
bool lockout_message_printed = false; // Flag to prevent multiple lockout messages
bool access_locked = false;  // Flag for master access lockout
time_t lockout_start_time = 0;  // Time when the lockout started


// Function to convert RFID tag ID to hexadecimal string
void convert_tag_to_string(uint8_t* tag_data, char* tag_string) {
    for (int i = 0; i < 12; ++i) {
        // Convert each byte to two hexadecimal digits and append to tag_string
        sprintf(tag_string + i * 2, "%02X", tag_data[i]);   
    }
    tag_string[24] = '\0';  // Null-terminate the string
}

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

    curr_Byte = 0;  // Track number of bytes received
    // Timeout timer for when waiting for tag to be scanned (to be added or removed)
    Timer timer;
    timer.start();

    while (true) {
        // If waiting for a tag to be scanned for 10 seconds or longer (prevent getting stuck)
        if (timer.elapsed_time().count() > 10000000) {
            printf("\nTag scan timed out\nReturning to menu...\n");
            return;
        }

        if (serial_port.readable()) {
            int Byte = serial_port.read(&add_data[curr_Byte], 1);
            if (Byte > 0) {
                curr_Byte++;

                // Convert tag_data[] to a hexadecimal string     
                if (curr_Byte >= 12) {
                    char tag_string[25];   // 12 bytes (2 hex digits per byte + null terminator)
                    convert_tag_to_string(add_data, tag_string);

                    // Master tag should can not be added
                    const char master_tag[] = "023341303036424537464334";
                    if (isAdding && strcmp(tag_string, master_tag) == 0) {
                        printf("Error: Master tag cannot be added as a regular tag.\n");
                        return;
                    }

                    // For adding a tag
                    if (isAdding) {

                        // Clean the tag - remove any spaces
                        std::string clean_tag(tag_string);
                        clean_tag.erase(std::remove_if(clean_tag.begin(), clean_tag.end(), ::isspace), clean_tag.end());

                        // Can't add a tag that's already in the system (prevents duplicates)
                        if (rfid_set.find(clean_tag) != rfid_set.end()) {
                            printf("Error: Tag already exists.\n");
                            return;
                        }

                        // Prompt user to enter a name for the tag
                        char tag_name[50] = "";
                        printf("Enter a name for this tag: ");
                        fgets(tag_name, sizeof(tag_name), stdin);

                        // Flush buffer
                        while (getchar() != '\n' && getchar() != EOF);

                        // Combine the tag ID with it's associated name
                        std::string tag_entry = clean_tag + " " + tag_name;
                        // Add to list of stored tags
                        rfid_tags.push_back(tag_entry);
                        rfid_set.insert(clean_tag);

                        printf("\nTag added successfully: %s (%s)\n", tag_string, tag_name);

                        // Prepare all tags for writing to the SD card (add new lines)
                        std::string all_tags;
                        for (const auto &tag : rfid_tags) {
                            all_tags += tag + "\n";
                        }

                        // Convert string to a character buffer
                        std::vector<char> buffer(all_tags.begin(), all_tags.end());
                        buffer.push_back('\0');

                        // Write all tags to SD card
                        sd.write_file("RFID_Tags.txt", buffer.data(), false, false);
                    }   
                    
                    // For when removing a tag
                    else {
                        // Searches the first 24 characters on each tag for a matching ID (ignores the name)
                        auto it = std::find_if(rfid_tags.begin(), rfid_tags.end(),
                        [&](const std::string &entry) {
                            return entry.substr(0, 24) == tag_string;
                        });

                        // If no tag found
                        if (it != rfid_tags.end()) {
                            std::string tag_name = "Unknown";
                            size_t space_pos = it->find(' ');
                            // Extract the name of the found tag
                            if (space_pos != std::string::npos) {
                                tag_name = it->substr(space_pos + 1);
                            }

                            // Remove tag from all lists 
                            rfid_tags.erase(it);
                            rfid_set.erase(tag_string);  
                            printf("\nTag removed successfully: %s (%s)\n", tag_string, tag_name.c_str());

                            // Rebuild the tag list 
                            std::string all_tags;
                            for (const auto &tag : rfid_tags) {
                                all_tags += tag + "\n";
                            }

                            // Prepare list for writing to SD card
                            std::vector<char> buffer(all_tags.begin(), all_tags.end());
                            buffer.push_back('\0');

                            // Write updated tag list to SD card
                            sd.write_file("RFID_Tags.txt", buffer.data(), false, false);
                        } else {
                            printf("Tag not found.\n");
                        }
                    }
                    return;
                }
            }
        }
    }
}

void rename_tag() {
    printf("\nRename function selected\nPlease scan the RFID tag to rename...\n");

    curr_Byte = 0;  // Track number of bytes received
    Timer timer;   // Timer for inactivity 
    timer.start();

    while (true) {
        // Timeout if no tag is scanned within 10 seconds (prevents getting stuck)
        if (timer.elapsed_time().count() > 10000000) {
            printf("\nTag scan timed out\nReturning to menu...\n");
            return;
        }

        // Read tag to be renamed
        if (serial_port.readable()) {
            int Byte = serial_port.read(&add_data[curr_Byte], 1);
            if (Byte > 0) {
                curr_Byte++;
                
                // Convert tag_data[] to a hexadecimal string
                if (curr_Byte >= 12) {
                    char tag_string[25];  // 12 bytes (2 hex digits per byte + null terminator)
                    convert_tag_to_string(add_data, tag_string);

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
                        fgets(new_name, sizeof(new_name), stdin);  // Read new name

                        // Flush input buffer (prevents invalid inputs next time user types)
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
                        // If invalid tag is scanned
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

    while(true) {
        // Check if there was a timeout since last incorrect attempt
        if (attempt_count >= 3) {
            time_t current_time = time(NULL);
            double seconds_elapsed = difftime(current_time, lockout_start_time);

            if (seconds_elapsed < 10) {
                // If less than 30 seconds have passed since the last attempt, deny access
                if (!lockout_message_printed) {
                    printf("\nAccess Locked. Please wait before trying again.\n");
                    access_locked = true;  // Lock access
                    lockout_message_printed = true; // Flag the message as printed
                }
                // Skip the password check, just continue the loop until timeout
                return;  // Recheck the timeout condition in the next loop iteration
            } else {
                // Reset the counter after timeout
                attempt_count = 0;
                lockout_message_printed = false; // Reset the flag after the lockout period
                access_locked = false;  // Unlock access
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
            return;  // Exit function
        }

        // If user enters correct password 
        if (strcmp(user_input, "jarman") == 0) {
            printf("\n\nMaster Card Access Granted");
            while(true){
                // Provide user with functions to add, remove and rename tags or to exit master access
                printf("\nPlease choose a function...\nType 'add' to add a new RFID tag to the system\nType 'remove' to remove a RFID tag from the system\nType 'rename' to rename a tag\nType 'exit' to exit master access\n");

                // Use fgets to read user input
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
                    printf("\nMaster access exited\n");
                    return;  // Exit function
                } 
                else {
                    // If the input doesn't match any of the above
                    printf("\nInvalid input\n");
                }
            }
        }
        else {
            // If the input doesn't match any of the above
            attempt_count++;  // Increment the incorrect password attempt count
            lockout_start_time = time(NULL);  // Record the time of the incorrect attempt
            printf("\nIncorrect Password\n");
            
            if (attempt_count >= 3) {
                // Action after 3 incorrect attempts
                esp32.write("T\n", 2); // Send the signal to the ESP32 to trigger email alert to phone
                printf("\nToo many incorrect attempts. Access locked for 10 seconds.\n");
                access_locked = true;  // Lock access
                return;
            }
        }
    }
}

void RFID_Read()
{
    if (serial_port.readable()) {
        // Read one byte from the RFID reader
        int Byte = serial_port.read(&tag_data[curr_Byte], 1);

        // To skip checking for a match when master card is scanned
        bool skip_match_check = false;

        // Check if the byte is valid
        if (Byte > 0) {
            curr_Byte++;  // Move to the next byte in the buffer
            
            // Check if we've received the full tag
            if (curr_Byte >= 12) {
                char tag_string[25];  // 12 bytes (2 hex digits per byte + null terminator)
                convert_tag_to_string(add_data, tag_string);

                char master[] = "023341303036424537464334"; // Master access tag ID for comparison

                if (strcmp(tag_string, master) == 0) {
                    skip_match_check = true;  // Set flag to skip checking the tag (master tag isn't in the system)
                    if (access_locked) {
                        printf("Access Denied\n");  // Deny access if locked
                    } else {
                        master_function();  // Proceed with master access
                    }
                }

                bool match = false;  

                // Check if scanned tag exists in the system - unordered_set for fast lookup
                if (rfid_set.find(tag_string) != rfid_set.end()) {
                    match = true;
                }

                // If the scanned tag is in the system
                if ((match == true) && (skip_match_check == false)){
                    // Find the tag's full entry in rfid_tags
                    std::string tag_name = "Unknown";

                    // Iterate through all stored tag entries to find a match           
                    for (const auto& entry : rfid_tags) {
                        if (entry.substr(0, 24) == tag_string) {  // Compare first 24 characters (tag ID) with scanned tag
                            size_t space_pos = entry.find(' ');   // Find position of first space (separates ID from name)
                            // If a space is found, extract the tag name from the entry
                            if (space_pos != std::string::npos) {
                                tag_name = entry.substr(space_pos + 1);  // Assign name
                            }
                            break;   // Exit the loop early since the matching tag has been found
                        }
                    }

                    // Print the tag ID and name
                    printf("Tag match found: %s (%s)\n", tag_string, tag_name.c_str());

                    // Get the current time from the RTC
                    rtc.get_time(&Time);
                    // Get the current date from the RTC
                    rtc.get_calendar(&Calendar);
                    
                    // Print the time and date the tag was scanned
                    printf("%02d/%02d/20%02d %02d:%02d:%02d\n", Calendar.date, Calendar.month, Calendar.year, Time.hours, Time.minutes, Time.seconds);
                }
                // If tag isn't in the sysetm
                else if((match == false) && (skip_match_check == false)){
                    printf("No matching tag found.\n");
                }
                else{
                    // Reset the flag to skip check if matser tag is scanned
                    skip_match_check = false;
                }
                // Reset the tag buffer for the next tag
                curr_Byte = 0;
            }
        }
    }
    
    if (access_locked) {
        // Calculate time elapsed since the master access lockout
        time_t current_time = time(NULL);
        double seconds_elapsed = difftime(current_time, lockout_start_time);
        if (seconds_elapsed >= 10) {
            access_locked = false;  // Unlock after timeout
        }
    }
}