/* ####################################################################################################################### */
/* # ____  ____  _     ___ _____   _____ _        _    ____    _____ ____  ____    __  __    _    ____ _____ _____ ____  # */
/* #/ ___||  _ \| |   |_ _|_   _| |  ___| |      / \  |  _ \  | ____/ ___||  _ \  |  \/  |  / \  / ___|_   _| ____|  _ \ # */
/* #\___ \| |_) | |    | |  | |   | |_  | |     / _ \ | |_) | |  _| \___ \| |_) | | |\/| | / _ \ \___ \ | | |  _| | |_) |# */
/* # ___) |  __/| |___ | |  | |   |  _| | |___ / ___ \|  __/  | |___ ___) |  __/  | |  | |/ ___ \ ___) || | | |___|  _ < # */
/* #|____/|_|   |_____|___| |_|   |_|   |_____/_/   \_|_|     |_____|____/|_|     |_|  |_/_/   \_|____/ |_| |_____|_| \_\# */
/* ####################################################################################################################### */
/*
  This project project is done for fun as part of: https://github.com/JonnyBooker/split-flap
  None of this would be possible without the brilliant work of David Königsmann: https://github.com/Dave19171/split-flap

  Licensed under GNU: https://github.com/JonnyBooker/split-flap/blob/master/LICENSE
*/

/* .--------------------------------------------------------------------------------. */
/* |  ___           __ _                    _    _       ___       __ _             | */
/* | / __|___ _ _  / _(_)__ _ _  _ _ _ __ _| |__| |___  |   \ ___ / _(_)_ _  ___ ___| */
/* || (__/ _ | ' \|  _| / _` | || | '_/ _` | '_ | / -_) | |) / -_|  _| | ' \/ -_(_-<| */
/* | \___\___|_||_|_| |_\__, |\_,_|_| \__,_|_.__|_\___| |___/\___|_| |_|_||_\___/__/| */
/* |                    |___/                                                       | */
/* '--------------------------------------------------------------------------------' */
/*
  These define statements can be changed as you desire for changing the functionality and
  behaviour of your device.
*/
#define SERIAL_ENABLE       false   //Option to enable serial debug messages
#define UNIT_CALLS_DISABLE  false   //Option to disable the call to the units so can just debug the ESP with no connections
#define OTA_ENABLE          true    //Option to enable OTA functionality
#define UNITS_AMOUNT        10      //Amount of connected units !IMPORTANT TO BE SET CORRECTLY!
#define SERIAL_BAUDRATE     115200  //Serial debugging BAUD rate
#define WIFI_SETUP_MODE     AP      //Option to either direct connect to a WiFi Network or setup a AP to configure WiFi. Options: AP or DIRECT

/* .--------------------------------------------------------. */
/* | ___         _               ___       __ _             | */
/* |/ __|_  _ __| |_ ___ _ __   |   \ ___ / _(_)_ _  ___ ___| */
/* |\__ | || (_-|  _/ -_| '  \  | |) / -_|  _| | ' \/ -_(_-<| */
/* ||___/\_, /__/\__\___|_|_|_| |___/\___|_| |_|_||_\___/__/| */
/* |     |__/                                               | */
/* '--------------------------------------------------------' */
/*
  These are important to maintain normal system behaviour. Only change if you know 
  what your doing.
*/
#define ANSWER_SIZE         1       //Size of unit's request answer
#define FLAP_AMOUNT         45      //Amount of Flaps in each unit
#define MIN_SPEED           1       //Min Speed
#define MAX_SPEED           12      //Max Speed
#define WEBSERVER_H                 //Needed in order to be compatible with WiFiManager: https://github.com/me-no-dev/ESPAsyncWebServer/issues/418#issuecomment-667976368

/* .-----------------------------------. */
/* | _    _ _                 _        | */
/* || |  (_| |__ _ _ __ _ _ _(_)___ ___| */
/* || |__| | '_ | '_/ _` | '_| / -_(_-<| */
/* ||____|_|_.__|_| \__,_|_| |_\___/__/| */
/* '-----------------------------------' */
/*
  External library dependencies, not much more to say!
*/

//WiFi Setup Library if we use that mode
//Specifically put here in this order to avoid conflict with other libraries
#if WIFI_SETUP_MODE == AP
#include <WiFiManager.h>
#endif

//OTA Libary if we are into that kind of thing
#if OTA_ENABLE == true
#include <ArduinoOTA.h>
#endif

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <ESP8266WiFi.h>
#include <ezTime.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include "Classes.h"
#include "LittleFS.h"
/* .------------------------------------------------------------------------------------. */
/* |  ___           __ _                    _    _       ___     _   _   _              | */
/* | / __|___ _ _  / _(_)__ _ _  _ _ _ __ _| |__| |___  / __|___| |_| |_(_)_ _  __ _ ___| */
/* || (__/ _ | ' \|  _| / _` | || | '_/ _` | '_ | / -_) \__ / -_|  _|  _| | ' \/ _` (_-<| */
/* | \___\___|_||_|_| |_\__, |\_,_|_| \__,_|_.__|_\___| |___\___|\__|\__|_|_||_\__, /__/| */
/* |                    |___/                                                  |___/    | */
/* '------------------------------------------------------------------------------------' */
/*
  Settings you can feel free to change to customise how your display works.
*/
//Used if connecting via "WIFI_SETUP_MODE" of "DIRECT" - Otherwise, leave blank
const char* wifiDirectSsid = "";
const char* wifiDirectPassword = "";

//Change if you want to have an Over The Air (OTA) Password for updates
const char* otaPassword = "";

//Change this to your timezone, use the TZ database name
//https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
const char* timezoneString = "Europe/London";

//If you want to have a different date or clock format change these two
//Complete table with every char: https://github.com/ropg/ezTime#getting-date-and-time
const char* dateFormat = "d.m.Y"; //Examples: d.m.Y -> 11.09.2021, D M y -> SAT SEP 21
const char* clockFormat = "H:i"; //Examples: H:i -> 21:19, h:ia -> 09:19PM

//How long to show a message for when a scheduled message is shown for
const int scheduledMessageDisplayTimeMillis = 7500;

/* .------------------------------------------------------------. */
/* | ___         _               ___     _   _   _              | */
/* |/ __|_  _ __| |_ ___ _ __   / __|___| |_| |_(_)_ _  __ _ ___| */
/* |\__ | || (_-|  _/ -_| '  \  \__ / -_|  _|  _| | ' \/ _` (_-<| */
/* ||___/\_, /__/\__\___|_|_|_| |___\___|\__|\__|_|_||_\__, /__/| */
/* |     |__/                                          |___/    | */
/* '------------------------------------------------------------' */
/*
  Used for normal running of the system so changing things here might make things 
  behave a little strange.
*/
//The current version of code to display on the UI
const char* espVersion = "2.0.0";

//All the letters on the units that we have to be displayed. You can change these if it so pleases at your own risk
const char letters[] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '$', '&', '#', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', '.', '-', '?', '!'};
int displayState[UNITS_AMOUNT];
unsigned long previousMillis = 0;

//Search for parameter in HTTP POST request
const char* PARAM_ALIGNMENT = "alignment";
const char* PARAM_FLAP_SPEED = "flapSpeed";
const char* PARAM_DEVICEMODE = "deviceMode";
const char* PARAM_INPUT_TEXT = "inputText";
const char* PARAM_SCHEDULE_ENABLED = "scheduleEnabled";
const char* PARAM_SCHEDULE_DATE_TIME = "scheduledDateTimeUnix";
const char* PARAM_COUNTDOWN_DATE = "countdownDateTimeUnix";
const char* PARAM_ID = "id";

//Device Modes
const char* DEVICE_MODE_TEXT = "text";
const char* DEVICE_MODE_CLOCK = "clock";
const char* DEVICE_MODE_DATE = "date";
const char* DEVICE_MODE_COUNTDOWN = "countdown";

//Alignment options
const char* ALIGNMENT_MODE_LEFT = "left";
const char* ALIGNMENT_MODE_CENTER = "center";
const char* ALIGNMENT_MODE_RIGHT = "right";

//File paths to save input values permanently
const char* alignmentPath = "/alignment.txt";
const char* flapSpeedPath = "/flapspeed.txt";
const char* deviceModePath = "/devicemode.txt";
const char* countdownPath = "/countdown.txt";
const char* scheduledMessagesPath = "/scheduled-messages.txt";

//Variables for storing things for checking and use in normal running
String alignment = "";
String flapSpeed = "";
String inputText = "";
String deviceMode = "";
String countdownToDateUnix = "";
String lastWrittenText = "";
String lastReceivedMessageDateTime = "";
bool alignmentUpdated = false;
bool isPendingReboot = false;
bool isPendingWifiReset = false;
bool isPendingUnitsReset = false;
LList<ScheduledMessage> scheduledMessages;
Timezone timezone; 

//Create AsyncWebServer object on port 80
AsyncWebServer webServer(80);

//Used for creating a Access Point to allow WiFi setup
#if WIFI_SETUP_MODE == AP
WiFiManager wifiManager;
bool isWifiConfigured = false;
#endif

//Used to denote that the system has gone into OTA mode
#if OTA_ENABLE == true
bool isInOtaMode = false;
#endif

/* .-----------------------------------------------. */
/* | ___          _          ___     _             | */
/* ||   \ _____ _(_)__ ___  / __|___| |_ _  _ _ __ | */
/* || |) / -_\ V | / _/ -_) \__ / -_|  _| || | '_ \| */
/* ||___/\___|\_/|_\__\___| |___\___|\__|\_,_| .__/| */
/* |                                         |_|   | */
/* '-----------------------------------------------' */
void setup() {
#if SERIAL_ENABLE == true
  //Setup so we can see serial messages
  Serial.begin(SERIAL_BAUDRATE);
#else
  //For ESP01 only
  Wire.begin(1, 3); 
  
  //De-activate I2C if debugging the ESP, otherwise serial does not work
  //Wire.begin(D1, D2); //For NodeMCU testing only SDA=D1 and SCL=D2
#endif
  SerialPrintln("");
  SerialPrintln("#######################################################");
  SerialPrintln("..............Split Flap Display Starting..............");
  SerialPrintln("#######################################################");

  //Load and read all the things
  initWiFi();
  
  //Helpful if want to force reset WiFi settings for testing
  //wifiManager.resetSettings();

  if (isWifiConfigured && !isPendingReboot) {
    //ezTime initialization
    waitForSync();
    timezone.setLocation(timezoneString);
    
    //Load various variables
    initialiseFileSystem();
    loadValuesFromFileSystem();

#if OTA_ENABLE == true
    SerialPrintln("OTA is enabled! Yay!");
#endif

    //Web Server Endpoint configuration
    webServer.serveStatic("/", LittleFS, "/");
    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
      SerialPrintln("Request Home Page Received");

      request->send(LittleFS, "/index.html", "text/html");
    });

    webServer.on("/settings", HTTP_GET, [](AsyncWebServerRequest * request) {
      SerialPrintln("Request for Settings Received");
      
      String json = getCurrentSettingValues();
      request->send(200, "application/json", json);
      json = String();
    });
    
    webServer.on("/health", HTTP_GET, [](AsyncWebServerRequest * request) {
      SerialPrintln("Request for Health Check Received");
      request->send(200, "text/plain", "Healthy");
    });
    
    webServer.on("/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
      SerialPrintln("Request to Reboot Received");
      
      //Create HTML page to explain the system is rebooting
      IPAddress ip = WiFi.localIP();
      
      String html = "<div style='text-align:center'>";
      html += "<font face='arial'><h1>Split Flap - Rebooting</h1>";
      html += "<p>Reboot is pending now...<p>";
      html += "<p>This can take anywhere between 10-20 seconds<p>";
      html += "<p>You can go to the main home page after this time by clicking the button below or going to '/'.</p>";
      html += "<p><a href=\"http://" + ip.toString() + "\">Home</a></p>";
      html += "</font>";
      html += "</div>";
      
      request->send(200, "text/html", html);
      isPendingReboot = true;
    });
    
    webServer.on("/reset-units", HTTP_GET, [](AsyncWebServerRequest * request) {
      SerialPrintln("Request to Reset Units Received");
      
      //This will be picked up in the loop
      isPendingUnitsReset = true;
      
      request->redirect("/?is-resetting-units=true");
    });

    webServer.on("/scheduled-message/remove", HTTP_DELETE, [](AsyncWebServerRequest * request) {
      SerialPrintln("Request to Remove Scheduled Message Received");
      
      if (request->hasParam(PARAM_ID)) {
        bool removedScheduledMessage = false;
        String idValue = request->getParam(PARAM_ID)->value();

        if (isNumber(idValue)) {
          long parsedIdValue = atol(idValue.c_str());
          bool removed = removeScheduledMessage(parsedIdValue);
          
          if (removed) {
            request->send(202, "text/plain", "Removed");
          }
          else {
            request->send(400, "text/plain", "Unable to find message with ID specified. Id: " + idValue);
          }
        }
        else {
          SerialPrintln("Invalid Delete Scheduled Message ID Received");
          request->send(400, "text/plain", "Invalid ID value");
        }
      } 
      else {
          SerialPrintln("Delete Scheduled Message Received with no ID");
          request->send(400, "text/plain", "No ID specified");
      }
    });

    webServer.on("/", HTTP_POST, [](AsyncWebServerRequest * request) {
      SerialPrintln("Request Post of Form Received");    

      bool submissionError = false;
      
      bool newMessageScheduleEnabledValue;
      long newMessageScheduleDateTimeUnixValue;
      String newAlignmentValue, newDeviceModeValue, newFlapSpeedValue, newInputTextValue, newCountdownToDateUnixValue;
      
      int params = request->params();
      for (int paramIndex = 0; paramIndex < params; paramIndex++) {
        AsyncWebParameter* p = request->getParam(paramIndex);
        if (p->isPost()) {
          //HTTP POST alignment value
          if (p->name() == PARAM_ALIGNMENT) {
            String receivedValue = p->value();
            if (receivedValue == ALIGNMENT_MODE_LEFT || receivedValue == ALIGNMENT_MODE_CENTER || receivedValue == ALIGNMENT_MODE_RIGHT) {
              newAlignmentValue = receivedValue;
            }
            else {
              SerialPrintln("Alignment provided was not valid. Value: " + receivedValue); 
              submissionError = true;
            }
          }

          //HTTP POST device mode value
          if (p->name() == PARAM_DEVICEMODE) {
            String receivedValue = p->value();
            if (receivedValue == DEVICE_MODE_TEXT || receivedValue == DEVICE_MODE_CLOCK || receivedValue == DEVICE_MODE_DATE || receivedValue == DEVICE_MODE_COUNTDOWN) {
              newDeviceModeValue = receivedValue;          
            }
            else {
              SerialPrintln("Device Mode provided was not valid. Invalid Value: " + receivedValue); 
              submissionError = true;
            }
          }

          //HTTP POST Flap Speed Slider value
          if (p->name() == PARAM_FLAP_SPEED) {
            newFlapSpeedValue = p->value().c_str();
          }

          //HTTP POST inputText value
          if (p->name() == PARAM_INPUT_TEXT) {
            newInputTextValue = p->value().c_str();
          }

          //HTTP POST Schedule Enabled
          if (p->name() == PARAM_SCHEDULE_ENABLED) {
            String newMessageScheduleEnabledString = p->value().c_str();
            newMessageScheduleEnabledValue = newMessageScheduleEnabledString == "on" ?
              true : 
              false;
          }

          //HTTP POST Schedule Seconds
          if (p->name() == PARAM_SCHEDULE_DATE_TIME) {
            String receivedValue = p->value().c_str();
            if (isNumber(receivedValue)) {
              newMessageScheduleDateTimeUnixValue = atol(receivedValue.c_str());
            }
            else {
              SerialPrintln("Schedule date time provided was not valid. Invalid Value: " + receivedValue); 
              submissionError = true;
            }
          }

          //HTTP POST Countdown Seconds
          if (p->name() == PARAM_COUNTDOWN_DATE) {
            String receivedValue = p->value().c_str();
            if (isNumber(receivedValue)) {
              newCountdownToDateUnixValue = receivedValue;
            }
            else {
              SerialPrintln("Countdown date provided was not valid. Invalid Value: " + receivedValue); 
              submissionError = true;
            }
          }
        }
      }    

      //If there was an error, report back to check what has been input
      if (submissionError) {
        SerialPrintln("Finished Processing Request with Error");
        request->redirect("/?invalid-submission=" + true);
      }
      else {
        SerialPrintln("Finished Processing Request Successfully");

        lastReceivedMessageDateTime = timezone.dateTime("d M y H:i:s");

        //Only if a new alignment value
        if (alignment != newAlignmentValue) {
          alignment = newAlignmentValue;
          alignmentUpdated = true;

          writeFile(LittleFS, alignmentPath, alignment.c_str());
          SerialPrintln("Alignment Updated: " + alignment);
        }

        //Only if a new flap speed value
        if (flapSpeed != newFlapSpeedValue) {
          flapSpeed = newFlapSpeedValue;

          writeFile(LittleFS, flapSpeedPath, flapSpeed.c_str());
          SerialPrintln("Flap Speed Updated: " + flapSpeed);
        }

        //Only if countdown date has changed
        if (countdownToDateUnix != newCountdownToDateUnixValue) {
          countdownToDateUnix = newCountdownToDateUnixValue;

          writeFile(LittleFS, countdownPath, countdownToDateUnix.c_str());
          SerialPrintln("Countdown Date Time Unix Updated: " + countdownToDateUnix);
        }

        //If its a new scheduled message, add it to the backlog and proceed, don't want to change device mode
        //Else, we do want to change the device mode and clear out the input text
        if (newMessageScheduleEnabledValue) {
          addAndPersistScheduledMessage(newInputTextValue, newMessageScheduleDateTimeUnixValue);
          SerialPrintln("New Scheduled Message added");
        }
        else {
          //Only if device mode has changed
          if (deviceMode != newDeviceModeValue) {
            deviceMode = newDeviceModeValue;

            writeFile(LittleFS, deviceModePath, deviceMode.c_str());
            SerialPrintln("Device Mode Set: " + deviceMode);
          }

          //Only if we are showing text
          if (deviceMode == DEVICE_MODE_TEXT) {
            inputText = newInputTextValue;
          }
        }

        //Redirect so that we don't have the "re-submit form" problem in browser for refresh
        request->redirect("/");
      }
    });

#if OTA_ENABLE == true
    webServer.on("/ota", HTTP_GET, [](AsyncWebServerRequest * request) {
      SerialPrintln("Request to start OTA mode received");
      
      //Create HTML page to explain OTA
      IPAddress ip = WiFi.localIP();
      
      String html = "<div style='text-align:center'>";
      html += "<font face='arial'><h1>Split Flap - OTA Update Mode</h1>";
      html += "<p>OTA mode has been started. You can now update your module via WiFI. Open your Arduino IDE and select the new port in \"Tools\" menu and upload the your sketch as normal!<p>";
      html += "<p>Open your Arduino IDE and select the new port in \"Tools\" menu and upload the your sketch as normal!</p>";
      html += "<p>After you have carried out your update, the system will automatically be rebooted. You can go to the main home page after this time by clicking the button below or going to '/'.</p>";
      html += "<p>You can take the system out of this mode by clicking the button to reboot below or going to '/reboot'.</p>";
      html += "<p><a href=\"http://" + ip.toString() + "\")\">Home</a></p>";
      html += "<p><a href=\"http://" + ip.toString() + "/reboot\")\">Reboot</a></p>";
      html += "</font>";
      html += "</div>";

      request->send(200, "text/html", html);
  
      if (!isInOtaMode) {
        SerialPrintln("Setting OTA Hostname");
        ArduinoOTA.setHostname("Split-Flap-OTA");

        //If there is a password set, disabled by default for ease
        if (otaPassword != "") {
          ArduinoOTA.setPassword(otaPassword);
        }
        
        SerialPrintln("Starting OTA Mode");
        ArduinoOTA.begin();
        delay(100);
      
        ArduinoOTA.onStart([]() {
          LittleFS.end();
          if (ArduinoOTA.getCommand() == U_FLASH) {
            SerialPrintln("Start updating sketch");
          } 
          else {
            SerialPrintln("Start updating filesystem");
          }  
        });
        
        ArduinoOTA.onEnd([]() {
          SerialPrintln("Finished OTA Update - Rebooting");
          isPendingReboot = true;
        });
        
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
          SerialPrintf("OTA Progress: %u%%\r", (progress / (total / 100)));
        });
        
        ArduinoOTA.onError([](ota_error_t error) {
          SerialPrintf("Error[%u]: ", error);

          if (error == OTA_AUTH_ERROR) {
            SerialPrintln("Finished OTA Update - Rebooting");
          }
          else if (error == OTA_BEGIN_ERROR) {
            SerialPrintln("OTA Begin Failed");
          }
          else if (error == OTA_CONNECT_ERROR) {
            SerialPrintln("OTA Connect Failed");
          }
          else if (error == OTA_RECEIVE_ERROR) {
            SerialPrintln("OTA Receive Failed");
          }
          else if (error == OTA_END_ERROR) {
            SerialPrintln("OTA End Failed");
          }
        });
        
        //Put in OTA Mode
        isInOtaMode = true;
      }
      else {
        SerialPrintln("Already in OTA Mode");
      }
    });
#endif

#if WIFI_SETUP_MODE == AP
    webServer.on("/reset-wifi", HTTP_GET, [](AsyncWebServerRequest * request) {
      SerialPrintln("Request to Reset WiFi Received");
      
      IPAddress ip = WiFi.localIP();
      
      String html = "<div style='text-align:center'>";
      html += "<font face='arial'><h1>Split Flap - Resetting WiFi</h1>";
      html += "<p>WiFi Settings have been erased. Device will now reboot...<p>";
      html += "<p>You will now be able to connect to this device in AP mode to configure the WiFi once more<p>";
      html += "<p>You can go to the main home page after this time by clicking the button below or going to '/'.</p>";
      html += "<p><a href=\"http://" + ip.toString() + "\">Home</a></p>";
      html += "</font>";
      html += "</div>";
      
      request->send(200, "text/html", html);
      isPendingWifiReset = true;
    });
#endif   

    delay(250);
    webServer.begin();

    SerialPrintln("Split Flap Ready!");
    SerialPrintln("#######################################################");
  }
  else {
    if (isPendingReboot) {
      SerialPrintln("Reboot is pending to be able to continue device function. Hold please...");
      SerialPrintln("#######################################################");
    }
    else {
      SerialPrintln("Unable to connect to WiFi... Not starting web server");
      SerialPrintln("Please hard restart your device to try connect again");
      SerialPrintln("#######################################################");
    }
  }
}

/* .----------------------------------------------------. */
/* | ___                _             _                 | */
/* || _ \_  _ _ _  _ _ (_)_ _  __ _  | |   ___ ___ _ __ | */
/* ||   | || | ' \| ' \| | ' \/ _` | | |__/ _ / _ | '_ \| */
/* ||_|_\\_,_|_||_|_||_|_|_||_\__, | |____\___\___| .__/| */
/* |                          |___/               |_|   | */
/* '----------------------------------------------------' */
void loop() {
  //Reboot in here as if we restart within a request handler, no response is returned
  if (isPendingReboot) {
    SerialPrintln("Rebooting Now... Fairwell!");
    SerialPrintln("#######################################################");
    delay(100);

    ESP.restart();
    return;
  }

  //Clear off the WiFi Manager Settings
  if (isPendingWifiReset) {
    SerialPrintln("Removing WiFi settings");
    wifiManager.resetSettings();
    delay(100);

    isPendingReboot = true;
    return;
  }

  //Do nothing if WiFi is not configured
  if (!isWifiConfigured) {
    //Show there is an error via text on display
    deviceMode = DEVICE_MODE_TEXT;
    alignment = ALIGNMENT_MODE_CENTER;
    flapSpeed = "80";

    showText("OFFLINE");
    delay(100);
    return;
  }

  if (isPendingUnitsReset) {
    SerialPrintln("Reseting Units now...");

    //Blank out the message
    String blankOutText1 = createRepeatingString('-');
    showText(blankOutText1);
    delay(2000);

    //Do just enough to do a full iteration which triggers the re-calibration
    String blankOutText2 = createRepeatingString('.');
    showText(blankOutText2);

    //We did a reset!
    isPendingUnitsReset = false;

    SerialPrintln("Done Units Reset!");
  }
  
#if OTA_ENABLE == true
  //If System is in OTA, try handle!
  if(isInOtaMode) {
    ArduinoOTA.handle();
    delay(1);
  }
#endif

  //ezTime library sync
  events(); 
  
  //Process every second
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;

    checkScheduledMessages();
    checkCountdown();

    //Mode Selection
    if (deviceMode == DEVICE_MODE_TEXT || deviceMode == DEVICE_MODE_COUNTDOWN) { 
      showText(inputText);
    } 
    else if (deviceMode == DEVICE_MODE_DATE) {
      showText(timezone.dateTime(dateFormat));
    } 
    else if (deviceMode == DEVICE_MODE_CLOCK) {
      showText(timezone.dateTime(clockFormat));
    } 
  }
}

//Gets all the currently stored calues from memory in a JSON object
String getCurrentSettingValues() {
  JsonDocument document;

  document["timezoneOffset"] = timezone.getOffset();
  document["unitCount"] = UNITS_AMOUNT;
  document["alignment"] = alignment;
  document["flapSpeed"] = flapSpeed;
  document["deviceMode"] = deviceMode;
  document["version"] = espVersion;
  document["lastTimeReceivedMessageDateTime"] = lastReceivedMessageDateTime;
  document["lastWrittenText"] = lastWrittenText;
  document["countdownToDateUnix"] = atol(countdownToDateUnix.c_str());

  for(int scheduledMessageIndex = 0; scheduledMessageIndex < scheduledMessages.size(); scheduledMessageIndex++) {
    ScheduledMessage scheduledMessage = scheduledMessages[scheduledMessageIndex];
    
    document["scheduledMessages"][scheduledMessageIndex]["scheduledDateTimeUnix"] = scheduledMessage.ScheduledDateTimeUnix;
    document["scheduledMessages"][scheduledMessageIndex]["message"] = scheduledMessage.Message;
  }

#if OTA_ENABLE == true
  document["otaEnabled"] = true;
  document["isInOtaMode"] = isInOtaMode;
#else
  document["otaEnabled"] = false;
#endif

#if WIFI_SETUP_MODE == AP
  document["wifiSettingsResettable"] = true;
#else
  document["wifiSettingsResettable"] = false;
#endif
  
  String jsonString;
  serializeJson(document, jsonString);

  return jsonString;
}
