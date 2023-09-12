//Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.hostname("SplitFlap");
  WiFi.begin(ssid, password);
  
  SerialPrint("Connecting to WiFi ..");

  while (WiFi.status() != WL_CONNECTED) {
    SerialPrint('.');
    delay(1000);
  }

  SerialPrintln(WiFi.localIP());
}
