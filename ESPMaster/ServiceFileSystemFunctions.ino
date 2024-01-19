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
  countdownToDateUnix = readFile(LittleFS, countdownPath, "0");
  alignment = readFile(LittleFS, alignmentPath, ALIGNMENT_MODE_LEFT);
  flapSpeed = readFile(LittleFS, flapSpeedPath, "80");
  deviceMode = readFile(LittleFS, deviceModePath, DEVICE_MODE_TEXT);

  String scheduledMessagesJson = readFile(LittleFS, scheduledMessagesPath, "");
  if (scheduledMessagesJson != "") {    
    SerialPrintln("Loading Scheduled Messages");
    readScheduledMessagesFromJson(scheduledMessagesJson);
  }
  
  SerialPrintln("Loaded Settings:");
  SerialPrintln("   Alignment: " + alignment);
  SerialPrintln("   Flap Speed: " + flapSpeed);
  SerialPrintln("   Device Mode: " + deviceMode);
  SerialPrintln("   Countdown to Date UNIX: " + countdownToDateUnix);
  SerialPrint("   Scheduled Message Count: ");
  SerialPrintln(scheduledMessages.size());
}

//Read File from LittleFS
String readFile(fs::FS &fs, const char * path, String defaultValue) {
  SerialPrintf("Reading file: %s\r\n", path);

  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    SerialPrintln("- Failed to open file for reading");

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
    SerialPrintln("- Failed to open file for writing");
    return;
  }
  
  if (file.print(message)) {
    SerialPrintln("- File written");
  } 
  else {
    SerialPrintln("- Write failed");
  }
}
