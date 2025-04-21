#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "mbed.h"

void onButtonPress();
void monitor_button();

extern Timer debounceTimer;

#endif