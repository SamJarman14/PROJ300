#ifndef _KEYPAD_H_
#define _KEYPAD_H_

#include "mbed.h"

// Functions
void scanKeypad();
void load_passcodes_from_sd();
void passcode_master_function();
extern DigitalIn gateSwitch;

#endif