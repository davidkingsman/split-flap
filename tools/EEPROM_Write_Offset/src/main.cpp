#include <Arduino.h>
#include <EEPROM.h>

const int offsetToWrite = 1;  // change this into the offset for this unit

const int eeAddress = 0;  //Location we want the data to be put.
int calOffsetGet;         //Variable already in EEPROM.

void writeToEEPROM(int intOffsetValue) {
    //One simple call, with the address first and the object second.
    EEPROM.put(eeAddress, intOffsetValue);

    Serial.print("Value written to EEPROM: ");
    Serial.print(intOffsetValue);
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    Serial.println("init");

    writeToEEPROM(offsetToWrite);
}

void loop() {
}