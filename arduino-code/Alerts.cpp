#include "Arduino.h"
#include "Alerts.h"
#include <HTTPClient.h>

void sendIFTTTAlert_Ped_MA() {
    HTTPClient http;
    http.begin("https://maker.ifttt.com/trigger/Pedestrian_Master_Access/with/key/kPjTMn6CZLRaN4xK1O49muVgoR1m022swTGm-es3Qqp");

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
    http.begin("https://maker.ifttt.com/trigger/Vehicle_Master_Access/with/key/kPjTMn6CZLRaN4xK1O49muVgoR1m022swTGm-es3Qqp");

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
    http.begin("https://maker.ifttt.com/trigger/Gate_Left_Open/with/key/kPjTMn6CZLRaN4xK1O49muVgoR1m022swTGm-es3Qqp");

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
    http.begin("https://maker.ifttt.com/trigger/Pedestrian_Passcodes/with/key/kPjTMn6CZLRaN4xK1O49muVgoR1m022swTGm-es3Qqp");

    int httpCode = http.GET();
    if (httpCode > 0) {
        Serial1.println("Alert sent!");
    } else {
        Serial1.println("Error sending alert.");
    }

    http.end();
}