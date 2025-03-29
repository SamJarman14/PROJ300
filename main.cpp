#include "mbed.h"
#include "Keypad.h"
#include "RFID.h"

Thread t1, t2;

int main() {
    printf("\nSystem Running...\n");
    load_tags_from_sd();   // Load active tags from SD card memory at startup
    t1.start(RFID_Read);
    t2.start(scanKeypad);
}