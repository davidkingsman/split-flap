#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <SocketIoClient.h>
#include <WiFiUdp.h>
#include <Wire.h>

#define UNITSAMOUNT 1  // !IMPORTANT! Amount of connected units, change this if you have a different amount of units connected

#define BAUDRATE 115200
#define ANSWERSIZE 1   // Size of units request answer
#define FLAPAMOUNT 45  // Amount of Flaps in each unit
#define MINSPEED 1     // min Speed
#define MAXSPEED 12    // max Speed
#define ESPLED 1       // Blue LED on ESP01
//#define serial         // uncomment for serial debug messages, no serial messages if this whole line is a comment!

struct deviceValue {
    const char* property;
    String value;
};

deviceValue deviceValues[4] = {
    {"alignment", "left"},
    {"speedSlider", "50"},
    {"deviceMode", "none"},
    {"inputText", "HELLOWORLD"}};

const int ALIGNMENT = 0;
const int SPEEDSLIDER = 1;
const int DEVICEMODE = 2;
const int INPUTTEXT = 3;

#include "config.h"
SocketIoClient webSocket;

const char* backendServer = "laptop.local";  //"splitflap.postduif.be";
const int backandPort = 3100;

// CHANGE THIS TO YOUR TIMEZONE, OFFSET IN SECONDS FROM GMT
//  GMT +1 = 3600
//  GMT +8 = 28800
//  GMT -1 = -3600
//  GMT 0 = 0
#define TIMEOFFSET 3600

const char letters[] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '$', '&', '#', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', '.', '-', '?', '!'};
int displayState[UNITSAMOUNT];
String writtenLast;
unsigned long previousMillis = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Week Days
String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Month names
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

String getSubstringValue(String inputData, char separator, int index) {
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = inputData.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (inputData.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }

    return (found > index) ? inputData.substring(strIndex[0], strIndex[1]) : "";
}

String getPayloadKey(String payload) {
    return getSubstringValue(payload, '|', 0);
}

String getPayloadValue(String payload) {
    return getSubstringValue(payload, '|', 1);
}

// converts input string to uppercase
String cleanString(String message) {
    message.toUpperCase();
    return message;
}

String centerString(String message) {
    String emptySpace;
    int messageLength = message.length();
    int emptySpaceAmount = (UNITSAMOUNT - messageLength) / 2;
    for (int i = 0; i < emptySpaceAmount; i++) {
        emptySpace = " " + emptySpace;
    }
    message = emptySpace + message;
    message = cleanString(message);
    return message;
}

String rightString(String message) {
    message = cleanString(message);
    return message;
}

// aligns string on left side of array and fills empty chars with spaces
String leftString(String message) {
    message = cleanString(message);

    char leftAlignString[UNITSAMOUNT + 1];

    for (int i = 0; i < UNITSAMOUNT + 1; i++) {
        if (i < message.length()) {
            leftAlignString[i] = message[i];
        } else if (i == UNITSAMOUNT) {
            leftAlignString[i] = '\0';
        } else {
            leftAlignString[i] = ' ';
        }
    }
#ifdef serial
    Serial.println(leftAlignString);
#endif
    return leftAlignString;
}

int convertSpeed(String speedSlider) {
    int speedSliderInt;
    speedSliderInt = speedSlider.toInt();
    speedSliderInt = map(speedSliderInt, 1, 100, MINSPEED, MAXSPEED);
    return speedSliderInt;
}

// translates char to letter position
int translateLettertoInt(char letterchar) {
    for (int i = 0; i < FLAPAMOUNT; i++) {
        if (letterchar == letters[i]) {
            return i;
        }
    }
    return 0;
}

// write letter position and speed in rpm to single unit
void writeToUnit(int address, int letter, int flapSpeed) {
    int sendArray[2] = {letter, flapSpeed};  // Array with values to send to unit

    Wire.beginTransmission(address);

    // Write values to send to slave in buffer
    for (int i = 0; i < sizeof sendArray / sizeof sendArray[0]; i++) {
#ifdef serial
        Serial.print("sendArray: ");
        Serial.println(sendArray[i]);
#endif
        Wire.write(sendArray[i]);
    }
    Wire.endTransmission();  // send values to unit
}

// checks if single unit is moving
int checkIfMoving(int address) {
    int active;
    Wire.requestFrom(address, ANSWERSIZE, true);
    active = Wire.read();
#ifdef serial
    Serial.print(address);
    Serial.print(":");
    Serial.print(active);
    Serial.println();
#endif
    if (active == -1) {
#ifdef serial
        Serial.println("Try to wake up unit");
#endif
        Wire.beginTransmission(address);
        Wire.endTransmission();
        // delay(5);
    }
    return active;
}

// checks if unit in display is currently moving
bool isDisplayMoving() {
    // Request all units moving state and write to array
    for (int i = 0; i < UNITSAMOUNT; i++) {
        displayState[i] = checkIfMoving(i);
        if (displayState[i] == 1) {
#ifdef serial
            Serial.println("A unit in the display is busy");
#endif
            return true;

            // if unit is not available through i2c
        } else if (displayState[i] == -1) {
#ifdef serial
            Serial.println("A unit in the display is sleeping");
#endif
            return true;
        }
    }
#ifdef serial
    Serial.println("Display is standing still");
#endif
    return false;
}

// pushes message to units
void showMessage(String message, int flapSpeed) {
    // Format string per alignment choice
    if (deviceValues[ALIGNMENT].value == "left") {
        message = leftString(message);
    } else if (deviceValues[ALIGNMENT].value == "right") {
        message = rightString(message);
    } else if (deviceValues[ALIGNMENT].value == "center") {
        message = centerString(message);
    }

    // wait while display is still moving
    while (isDisplayMoving()) {
#ifdef serial
        Serial.println("wait for display to stop");
#endif
        delay(500);
    }

#ifdef serial
    Serial.println(message);
#endif

    for (int i = 0; i < UNITSAMOUNT; i++) {
        char currentLetter = message[i];
        int currentLetterPosition = translateLettertoInt(currentLetter);
#ifdef serial
        Serial.print("Unit Nr.: ");
        Serial.print(i);
        Serial.print(" Letter: ");
        Serial.print(message[i]);
        Serial.print(" Letter position: ");
        Serial.println(currentLetterPosition);
#endif
        writeToUnit(i, currentLetterPosition, flapSpeed);
    }
}

// checks for new message to show
void showNewData(String message) {
    if (writtenLast != message) {
        showMessage(message, convertSpeed(deviceValues[SPEEDSLIDER].value));
    }
    writtenLast = message;
}

void showDate() {
    timeClient.update();

    unsigned long epochTime = timeClient.getEpochTime();

    String weekDay = weekDays[timeClient.getDay()];
#ifdef serial
    Serial.print("Week Day: ");
    Serial.println(weekDay);
#endif
    // Get a time structure
    struct tm* ptm = gmtime((time_t*)&epochTime);

    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon + 1;
    int currentYear = ptm->tm_year + 1900;

    String currentMonthName = months[currentMonth - 1];

    String currentDate;

    // Add leading zeroes
    if (monthDay < 10 && currentMonth < 10) {
        currentDate = "0" + String(monthDay) + "." + "0" + String(currentMonth) + "." + String(currentYear);
    } else if (currentMonth < 10) {
        currentDate = String(monthDay) + "." + "0" + String(currentMonth) + "." + String(currentYear);
    } else if (monthDay < 10) {
        currentDate = "0" + String(monthDay) + "." + String(currentMonth) + "." + String(currentYear);
    } else {
        currentDate = String(monthDay) + "." + String(currentMonth) + "." + String(currentYear);
    }
    showNewData(currentDate);
}

void showClock() {
    timeClient.update();

    unsigned long epochTime = timeClient.getEpochTime();

    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();

    String currentTime;

    // Add leading zero
    if (currentMinute < 10 && currentHour < 10) {
        currentTime = "0" + String(currentHour) + ":" + "0" + String(currentMinute);
    } else if (currentMinute < 10) {
        currentTime = String(currentHour) + ":" + "0" + String(currentMinute);
    } else if (currentHour < 10) {
        currentTime = "0" + String(currentHour) + ":" + String(currentMinute);
    } else {
        currentTime = String(currentHour) + ":" + String(currentMinute);
    }
    showNewData(currentTime);
}

void setupTime() {
    // Needed for time functions
    timeClient.begin();
    timeClient.setTimeOffset(TIMEOFFSET);
#ifdef serial
    Serial.println("timeClient initialized");
#endif
}

String serialPrintPayload(const char* payload) {
    String receivedText = String(payload);
#ifdef serial
    Serial.print("received: ");
    Serial.println(receivedText);
#endif
    return receivedText;
}

void emitMessage(const char* event, const char* message) {
    char payload[255];
    sprintf(payload, "\"%s\"", message);

    webSocket.emit(event, payload);
}

void handleConnect(const char* payload, size_t length) {
    serialPrintPayload(payload);
    emitMessage("identify", unitID);
}

void emitValues() {
    char payload[255];
    char valueString[255];
    char* valuesTemplate = "{%s}";
    char* valueTemplate = "\"%s\":\"%s\"";

    strcpy(payload, "");
    strcpy(valueString, "");

    for (int i = 0; i < 4; i++) {
        sprintf(valueString, valueTemplate, deviceValues[i].property, deviceValues[i].value);
        if (strlen(payload) > 0) {
            strcat(payload, ",");
        }
        strcat(payload, valueString);
    }

    sprintf(valueString, valuesTemplate, payload);

    webSocket.emit("valuesUpdate", valueString);
}

void handleMessage(const char* payload, size_t length) {
    String receivedText = serialPrintPayload(payload);
    String action = getPayloadKey(receivedText);
    String value = getPayloadValue(receivedText);

    if (action == "getValues") {
        emitValues();
    } else {
        if (action == "setAlignment") {
            deviceValues[ALIGNMENT].value = value;
        } else if (action == "setSpeedSlider") {
            deviceValues[SPEEDSLIDER].value = value;
        } else if (action == "setDeviceMode") {
            deviceValues[DEVICEMODE].value = value;
        } else if (action == "setInputText") {
            deviceValues[INPUTTEXT].value = value;
        }
        emitValues();
    }
    
#ifdef serial
    Serial.print("receivedText: ");
    Serial.println(receivedText);
    Serial.print("action: ");
    Serial.println(action);
    Serial.print("value: ");
    Serial.println(value);
#endif
}

void setup() {
#ifdef serial
    Serial.begin(BAUDRATE);
    Serial.println("");
    Serial.println("master start");

    Serial.print("Connecting to ");
    Serial.println(ssid);
#endif

    // WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
#ifdef serial
        Serial.print(".");
#endif
    }

#ifdef serial
    Serial.println("WiFi connected!!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
#endif

// deactivate I2C if debugging the ESP, otherwise serial does not work
#ifndef serial
    Wire.begin(1, 3);  // For ESP01 only
#endif

#ifdef serial
    Wire.begin(D1, D2);
#endif

    setupTime();  // initializes ntp function

    webSocket.on("message", handleMessage);
    webSocket.on("connect", handleConnect);
    webSocket.begin(backendServer, backandPort);
}

void loop() {
    webSocket.loop();

    //Reset loop delay
    unsigned long currentMillis = millis();

    //Delay to not spam web requests
    if (currentMillis - previousMillis >= 1024) {
        previousMillis = currentMillis;

        //Mode Selection
        if (deviceValues[DEVICEMODE].value == "text") {
#ifdef serial
            Serial.println("text");
            Serial.println(deviceValues[INPUTTEXT].value);
#endif
            showNewData(deviceValues[INPUTTEXT].value);
        } else if (deviceValues[DEVICEMODE].value == "date") {
            showDate();
        } else if (deviceValues[DEVICEMODE].value == "clock") {
            showClock();
        }
    }
}