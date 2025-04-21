#include <WiFi.h>
#include <HTTPClient.h>
#include "Ultrasonic_Sensors.h"
#include "Motors.h"
#include "Alerts.h"
#include "wifi_connect.h"
#include "Arduino.h"

const int ledPin = 2;  // GPIO2 for built-in LED

#define RX_PIN 16  // RX 
#define TX_PIN 17  // TX

void setup() {
  Serial.begin(115200);  // Initialise the serial monitor
  Serial1.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);  // Initialise UART

  pinMode(ledPin, OUTPUT);  // Set LED pin as output

  // Set Motor pins as outputs
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  // Set up PWM for motors
  ledcAttach(PWMA, pwmFreq, pwmResolution); 
  ledcAttach(PWMB, pwmFreq, pwmResolution); 

  // Set initial speed to 0
  ledcWrite(PWMA, 0); 
  ledcWrite(PWMB, 0); 

  // Set the Trigger and Echo pins as output and input respectively
  pinMode(TRIGGER_PIN_A, OUTPUT);
  pinMode(ECHO_PIN_A, INPUT);
  pinMode(TRIGGER_PIN_B, OUTPUT);
  pinMode(ECHO_PIN_B, INPUT);

  Set up a serial data receiver interrupt
  Serial1.onReceive(serialEvent);

}

void serialEvent() {
  while (Serial1.available() > 0) {
    char entry = 'X';                // Default value - in case avgDist_A == avgDist_B
    char incomingChar = Serial1.read();

    if (incomingChar == 'O') {            // If character to open the vehicle gate is received
      float avgDist_A = ULT_SEN_A();      // Get ultrasonic sensor A distance
      float avgDist_B = ULT_SEN_B();      // Get ultrasonic sensor B distance

      if (avgDist_A < avgDist_B) {
        entry = 'Y';
        Serial1.write('Y');              // Send signal to Nucleo that an entry was detected
        Serial.println("Entry");
      } 
      else if (avgDist_B < avgDist_A) {
        entry = 'N';
        Serial1.write('N');              // Send signal to Nucleo that an exit was detected
        Serial.println("Exit");
      }

      MotorA_Forward();
      delay(575);
      MotorA_Stop();   

      if (entry == 'Y') {
        while (true) {
          float avgDist_B = ULT_SEN_B();         // Get ultrasonic sensor B distance

          if (avgDist_B < 20) {                  // Object detected
            // Wait until the object moves away
            while (ULT_SEN_B() < 20) {
              delay(100);                        // Avoid excessive polling
            }

            MotorA_Reverse();
            delay(575);
            MotorA_Stop();

            Serial1.write('C');                  // Send signal over to Nucleo that the gate closing is complete
            break;                               // Exit the loop
          }
        }
      }

      if (entry == 'N') {
        while (true) {
          float avgDist_A = ULT_SEN_A();         // Get ultrasonic sensor A distance

          if (avgDist_A < 20) {                  // Object detected
            // Wait until the object moves away
            while (ULT_SEN_A() < 20) {
              delay(100);                        // Avoid excessive polling
            }

            MotorA_Reverse();
            delay(575);
            MotorA_Stop();

            Serial1.write('C');                  // Send signal over to Nucleo that the gate closing is complete
            break;                               // Exit the loop
          }
        }
      }
    }
    else if (incomingChar == 'G') {             // If the trigger character is received for opening the pedestrian gate 
      MotorB_Forward();
      delay(62);
      MotorB_Stop();
      Serial1.write('C');                      // Send signal over to Nucleo that the gate opening is complete
    }
    else if (incomingChar == 'C') {            // If the trigger character is received for closing the pedestrian gate 
      MotorB_Reverse();
      delay(62);
      MotorB_Stop();
    }
    else if (incomingChar == 'T') {            // If the trigger character is received for 3 incorrect master access password attempts (pedestrian gate)
      Serial1.println("Trigger received");
      void WiFi_Connect();
      sendIFTTTAlert_Ped_MA();
      WiFi.disconnect(true);
    }
    else if (incomingChar == 'L') {           // If the trigger character is received for the pedestrian gate being left open
      Serial1.println("Trigger received");
      void WiFi_Connect();
      sendIFTTTAlert_Gate_Open();
      WiFi.disconnect(true);
    }
    else if (incomingChar == 'M') {          // If the trigger character is received for 3 incorrect master access password attempts (vehicle gate)
      Serial1.println("Trigger received");
      void WiFi_Connect();
      sendIFTTTAlert_Veh_MA();
      WiFi.disconnect(true);
    }
    else if (incomingChar == 'I') {          // If the trigger character is received for 3 incorrect keypad passcode attempts
      Serial1.println("Trigger received");
      void WiFi_Connect();
      sendIFTTTAlert_Inc_Pasc();
      WiFi.disconnect(true);
    }
  }
}