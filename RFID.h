#ifndef _RFID_H_
#define _RFID_H_

#include "mbed.h"
#include "ds3231.h"
#include "SDCard.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <chrono>
#include <sstream>
#include <unordered_set>

// Functions
void load_tags_from_sd();
void process_tag(bool isAdding);
void rename_tag();
void master_function();
void RFID_Read();

// Global mutex lock for UART
extern Mutex uart_mutex;

// Global variables
extern UnbufferedSerial esp32;  
extern UnbufferedSerial serial_port; 
extern DigitalOut cs;

// Global use of SD card
extern SDCard sd;           // MOSI, MISO, SCLK, CS

// Global DS3231 usage
extern Ds3231 rtc; 
extern ds3231_time_t Time;
extern ds3231_calendar_t Calendar; 

#endif