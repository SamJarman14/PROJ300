#include "Arduino.h"
#include "wifi_connect.h"
#include <WiFi.h>

// WiFi credentials
const char* ssid = "SamJ";  // Wi-Fi ssid
const char* password = "samjarman";  // password

void WiFi_Connect(){
  WiFi.disconnect(true);  // Ensure a fresh connection
  delay(1000);

  WiFi.mode(WIFI_STA);  // Set ESP32 to station mode

  WiFi.begin(ssid, password);  

  Serial.print("Connecting to WiFi");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
      Serial.print("Status: ");
      Serial.println(WiFi.status());  // Debug - Print the Wi-Fi connection status code
  }

  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}