#include "Arduino.h"
#include "Ultrasonic_Sensors.h"

float ULT_SEN_A() {
  float totalDistance = 0;
  int numReadings = 5;

  for (int i = 0; i < numReadings; i++) {
    // Send a pulse to trigger the sensor
    digitalWrite(TRIGGER_PIN_A, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN_A, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN_A, LOW);

    // Measure the duration of the pulse coming from the Echo pin
    long duration = pulseIn(ECHO_PIN_A, HIGH);

    // Calculate the distance (in cm)
    float distance = duration * 0.0344 / 2;
    
    // Accumulate the distance
    totalDistance += distance;

    // Small delay between readings to avoid sensor interference
    delay(50);
  }

  // Calculate the average distance
  float avgDistance = totalDistance / numReadings;

  // Print the average distance
  Serial.print("Distance A: ");
  Serial.print(avgDistance);
  Serial.println(" cm");

  // Return the average distance
  return avgDistance;
}


float ULT_SEN_B() {
  float totalDistance = 0;
  int numReadings = 5;

  for (int i = 0; i < numReadings; i++) {
    // Send a pulse to trigger the sensor
    digitalWrite(TRIGGER_PIN_B, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN_B, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN_B, LOW);

    // Measure the duration of the pulse coming from the Echo pin
    long duration = pulseIn(ECHO_PIN_B, HIGH);

    // Calculate the distance (in cm)
    float distance = duration * 0.0344 / 2;
    
    // Accumulate the distance
    totalDistance += distance;

    // Small delay between readings to avoid sensor interference
    delay(50);
  }

  // Calculate the average distance
  float avgDistance = totalDistance / numReadings;

  // Print the average distance
  Serial.print("Distance B: ");
  Serial.print(avgDistance);
  Serial.println(" cm");

  // Return the average distance
  return avgDistance;
}