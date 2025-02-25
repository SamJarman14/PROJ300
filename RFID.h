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


#endif