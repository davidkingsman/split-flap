#include <Arduino.h>
#include <Wire.h>

#define DISPLAY_ERROR false
#define LOOP_DELAY 5000
#define USER_PIN true

// Customize I2C bus pins for ESP8266 or ESP32
const int PIN_SCL = D1;
const int PIN_SDA = D2;

String I2Ctest() {
    byte error, address;
    int nDevices;
    String s;

    s = "Scanning...\n";

    nDevices = 0;
    for (address = 0; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            s += "I2C device found at 0x";
            if (address < 16)
                s += "0";
            s += String(address, HEX);
            s += "\n";

            nDevices++;
        } else if (error > 0) {
            if (DISPLAY_ERROR) {
                s += "Unknow error at 0x";
                if (address < 16)
                    s += "0";
                s += String(address, HEX);
                s += "\n";
            }
        }
    }
    if (nDevices == 0)
        s += "No I2C devices found\n";
    else
        s += "done\n";
    return s;
}

void setup() {
    Serial.begin(115200);
    Serial.println("I2C scanner");
#if USER_PIN
    Wire.begin(PIN_SDA, PIN_SCL);
#else
    Wire.begin();
#endif
}

void loop() {
    // put your main code here, to run repeatedly:
    Serial.println(I2Ctest());
    delay(LOOP_DELAY);
}