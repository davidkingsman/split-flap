void loadFSValues() {
  // Load values saved in LittleFS
  alignment = readFile(LittleFS, alignmentPath);
  speedslider = readFile(LittleFS, speedsliderPath);
  devicemode = readFile(LittleFS, devicemodePath);
}

String getCurrentInputValues() {
  values["alignment"] = alignment;
  values["speedSlider"] = speedslider;
  values["devicemode"] = devicemode;

  String jsonString = JSON.stringify(values);
  return jsonString;
}


// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
#ifdef serial
  Serial.print("Connecting to WiFi ..");
#endif
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
#ifdef serial
  Serial.println(WiFi.localIP());
#endif
}


// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}


// Read File from LittleFS
String readFile(fs::FS &fs, const char * path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
}


// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
}
