#include "Arduino.h"
#include "Motors.h"

void MotorA_Forward(){
  ledcWrite(PWMA, 75);        // Set speed from 0 (stop) to 255 (full speed)
  digitalWrite(AIN1, HIGH);   // Set direction
  digitalWrite(AIN2, LOW);    // 
}

void MotorA_Reverse(){
  ledcWrite(PWMA, 75);         // Set speed from 0 (stop) to 255 (full speed)
  digitalWrite(AIN1, LOW);     // Set direction
  digitalWrite(AIN2, HIGH);    //
}

void MotorA_Stop(){
  ledcWrite(PWMA, 0);          // Stop motor
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
}

void MotorB_Forward(){
  ledcWrite(PWMB, 255);         // Set speed from 0 (stop) to 255 (full speed)
  digitalWrite(BIN1, HIGH);     // Set direction
  digitalWrite(BIN2, LOW);      //
}

void MotorB_Reverse(){
  ledcWrite(PWMB, 255);        // Set speed from 0 (stop) to 255 (full speed)
  digitalWrite(BIN1, LOW);     // Set direction
  digitalWrite(BIN2, HIGH);    //
}

void MotorB_Stop(){
  ledcWrite(PWMB, 0);          // Stop motor
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
}