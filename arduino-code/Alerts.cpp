#include "Arduino.h"
#include "Alerts.h"
#include <HTTPClient.h>

void sendIFTTTAlert_Ped_MA() {
    HTTPClient http;
    http.begin("https://maker.ifttt.com/trigger/Pedestrian Master Access/with/key/ftNHaGmQYdqgmhMbwAOb7pBOOWrPxy8-55Har2kYnVy");

    int httpCode = http.GET();
    if (httpCode > 0) {
        Serial1.println("Alert sent!");
    } else {
        Serial1.println("Error sending alert.");
    }

    http.end();
}

void sendIFTTTAlert_Veh_MA() {
    HTTPClient http;
    http.begin("https://maker.ifttt.com/trigger/Vehicle Master Access /with/key/ftNHaGmQYdqgmhMbwAOb7pBOOWrPxy8-55Har2kYnVy");

    int httpCode = http.GET();
    if (httpCode > 0) {
        Serial1.println("Alert sent!");
    } else {
        Serial1.println("Error sending alert.");
    }

    http.end();
}

void sendIFTTTAlert_Gate_Open() {
    HTTPClient http;
    http.begin("https://maker.ifttt.com/trigger/Pedestrian Gate Left Open/with/key/ftNHaGmQYdqgmhMbwAOb7pBOOWrPxy8-55Har2kYnVy");

    int httpCode = http.GET();
    if (httpCode > 0) {
        Serial1.println("Alert sent!");
    } else {
        Serial1.println("Error sending alert.");
    }

    http.end();
}

void sendIFTTTAlert_Inc_Pasc() {
    HTTPClient http;
    http.begin("https://maker.ifttt.com/trigger/Pedestrian Gate Keypad Passcode/with/key/ftNHaGmQYdqgmhMbwAOb7pBOOWrPxy8-55Har2kYnVy");

    int httpCode = http.GET();
    if (httpCode > 0) {
        Serial1.println("Alert sent!");
    } else {
        Serial1.println("Error sending alert.");
    }

    http.end();
}