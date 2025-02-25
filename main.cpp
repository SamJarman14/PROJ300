#include "mbed.h"
#include "getdistance.h"
#include "motor.h"
#include "Keypad.h"
#include "RFID.h"

//Motor Wheel(D13,D11,D9,D10);   //Instance of the Motor Class called 'Wheel' see motor.h and motor.cpp

//Variable 'duty' to vary speed as required 
//float duty = 0.9;


int main() {

    printf("\nSystem Running...\n");
    
    load_tags_from_sd();   // Load active tags from SD card memory at startup

    while(true) {

        RFID_Read();

        // Wheel.Speed(duty,duty);
        // wait_us(500000);
        // Wheel.Speed(0,0);
        // wait_us(1000000);

        //scanKeypad();
        //wait_us(100000); // Debounce delay

        // int dist = getdistance()/10;
        // printf("distance is %dcm\n", dist);
        // wait_us(100000);
    }
}