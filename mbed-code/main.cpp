#include "mbed.h"
#include "Keypad.h"
#include "RFID.h"
#include "Button.h"
#include <cstdint>

Thread t1, t2;

int main() {
    printf("\nSystem Running...\n");
    load_tags_from_sd();           // Load active tags from SD card memory at startup
    load_passcodes_from_sd();      // Load active passcodes from SD card memory at startup
    t1.start(RFID_Read);
    t2.start(scanKeypad);2
    +++++++++++++++++++++++++++++

    debounceTimer.start();         // Start the timer

    while (true) {
        monitor_button();
        ThisThread::sleep_for(250ms);
    }
}