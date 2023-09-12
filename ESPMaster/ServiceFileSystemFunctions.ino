//Initialize LittleFS
void initialiseFileSystem() {
  if (!LittleFS.begin()) {
    SerialPrintln("An error has occurred while mounting LittleFS");
  }

  SerialPrintln("LittleFS mounted successfully");
}

//Load variables that are saved to the file system
void loadValuesFromFileSystem() {
  //Load values saved in LittleFS
  alignment = readFile(LittleFS, alignmentPath, ALIGNMENT_MODE_LEFT);
  flapSpeed = readFile(LittleFS, flapSpeedPath, "80");
  currentDeviceMode = readFile(LittleFS, deviceModePath, DEVICE_MODE_TEXT);
  previousDeviceMode = currentDeviceMode;

  SerialPrintln("Alignment: " + alignment);
  SerialPrintln("Flap Speed: " + flapSpeed);
  SerialPrintln("Device Mode: " + currentDeviceMode);
}

//Gets all the currently stored calues from memory in a JSON object
String getCurrentSettingValues() {
  JSONVar values;

  values["alignment"] = alignment;
  values["flapSpeed"] = flapSpeed;
  values["deviceMode"] = currentDeviceMode;
  values["version"] = espVersion;
  values["unitCount"] = UNITSAMOUNT;
  values["lastTimeReceivedMessage"] = lastReceivedMessageDateTime;
  values["lastInputMessage"] = inputText;

  for(int scheduledMessageIndex = 0; scheduledMessageIndex < scheduledMessages.size(); scheduledMessageIndex++) {
    ScheduledMessage scheduledMessage = scheduledMessages[scheduledMessageIndex];
    
    values["scheduledMessages"][scheduledMessageIndex]["scheduledDateTimeMillis"] = scheduledMessage.ScheduledDateTimeMillis;
    values["scheduledMessages"][scheduledMessageIndex]["message"] = scheduledMessage.Message;
  }
  
  String jsonString = JSON.stringify(values);
  return jsonString;
}

//Read File from LittleFS
String readFile(fs::FS &fs, const char * path, String defaultValue) {
  SerialPrintf("Reading file: %s\r\n", path);

  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    SerialPrintln("- failed to open file for reading");

    if (defaultValue.length() > 0) {
      SerialPrintln("- Writing a default value as one has been specified. Default: " + defaultValue);
      writeFile(LittleFS, path, defaultValue.c_str());
      
      return defaultValue;
    }
    
    return String();
  }

  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
}

//Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message) {
  SerialPrintf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if (!file) {
    SerialPrintln("- failed to open file for writing");
    return;
  }
  
  if (file.print(message)) {
    SerialPrintln("- file written");
  } 
  else {
    SerialPrintln("- frite failed");
  }
}
