#ifndef MOTORS_H
#define MOTORS_H

void MotorA_Forward();
void MotorA_Reverse();
void MotorA_Stop();
void MotorB_Forward();
void MotorB_Reverse();
void MotorB_Stop();

#define PWMA 27   // Use GPIO 27 for PWM
#define AIN1 25   // Direction motor A pin 1
#define AIN2 26   // Direction motor A pin 2

#define PWMB 21   // Use GPIO 21 for PWM
#define BIN1 18   // Direction motor B pin 1
#define BIN2 19   // Direction motor B pin 2  

const int pwmFreq = 5000;  // PWM frequency in Hz
const int pwmResolution = 8;  // 8-bit resolution (0 to 255)
const int pwmChannelA = 0;  // Channel for Motor A PWM

#endif