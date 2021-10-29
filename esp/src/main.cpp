#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <SocketIoClient.h>
#include <WiFiUdp.h>
#include <Wire.h>

#define UNITSAMOUNT 10  // !IMPORTANT! Amount of connected units, change this if you have a different amount of units connected

#define BAUDRATE 115200
#define ANSWERSIZE 1   // Size of units request answer
#define FLAPAMOUNT 45  // Amount of Flaps in each unit
#define MINSPEED 1     // min Speed
#define MAXSPEED 12    // max Speed
#define ESPLED 1       // Blue LED on ESP01
#define serial         // uncomment for serial debug messages, no serial messages if this whole line is a comment!

#include "config.h"
SocketIoClient webSocket;

const char* backendServer = "splitflap.postduif.be";
const int backandPort = 80;

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

// Variables to save values from HTML form
String alignment;
String speedslider;
String devicemode;
String input1;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Week Days
String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
// Month names
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

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
    // Serial.println(leftAlignString);
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
    if (alignment == "left") {
        message = leftString(message);
    } else if (alignment == "right") {
        message = rightString(message);
    } else if (alignment == "center") {
        message = centerString(message);
    }

    // wait while display is still moving
    while (isDisplayMoving()) {
#ifdef serial
        Serial.println("wait for display to stop");
#endif
        delay(500);
    }

    Serial.println(message);
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
        showMessage(message, convertSpeed(speedslider));
    }
    writtenLast = message;
}

void showDate() {
    timeClient.update();

    unsigned long epochTime = timeClient.getEpochTime();

    String weekDay = weekDays[timeClient.getDay()];
    // Serial.print("Week Day: ");
    // Serial.println(weekDay);

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

void serialPrintPayload(const char* payload) {
    String receivedText = String(payload);
    Serial.print("received: ");
    Serial.println(receivedText);
}

void handleConnect(const char* payload, size_t length) {
    serialPrintPayload(payload);
}

void handleMessage(const char* payload, size_t length) {
    serialPrintPayload(payload);
}

void setup() {
    Serial.begin(115200);
    delay(10);

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    // WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("WiFi connected!!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    webSocket.on("update", handleMessage);
    webSocket.on("connect", handleConnect);
    webSocket.begin(backendServer, backandPort);

#ifdef serial
    Serial.begin(BAUDRATE);
    Serial.println("master start");
#endif

// deactivate I2C if debugging the ESP, otherwise serial does not work
#ifndef serial
    Wire.begin(1, 3);  // For ESP01 only
#endif

    setupTime();  // initializes ntp function
}

void loop() {
    webSocket.loop();
}