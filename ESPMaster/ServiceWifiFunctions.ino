//Initialize WiFi
void initWiFi() {
  int wifiConnectTimeoutSeconds = 180;

  WiFi.mode(WIFI_STA);

#if WIFI_SETUP_MODE == AP
  SerialPrintln("Setting up WiFi AP Setup Mode");

  wifiManager.setTitle("Split-Flap Setup");
  wifiManager.setHostname("Split-Flap");
  wifiManager.setDarkMode(true);
  wifiManager.setShowInfoUpdate(false);
  wifiManager.setConfigPortalBlocking(true);
  wifiManager.setConfigPortalTimeout(wifiConnectTimeoutSeconds);
  wifiManager.setConnectTimeout(120);    
  wifiManager.setWiFiAutoReconnect(true);
  wifiManager.setSaveConfigCallback([]() {
    //Sadly, if we've had to open up the WiFi manager portal to set the WiFi configuration up
    //then a soft reset is necessary. There looks to be issues with the ESP and running
    //a webserver alongside the WiFiManager. Some reading around this issue here suggests it is
    //not possible to fix:
    //https://github.com/tzapu/WiFiManager/issues/1579
    
    //Suggestion to fix is reset the device:
    //https://github.com/rancilio-pid/clevercoffee/issues/323#issuecomment-1587344185

    SerialPrintln("New WiFi configuration saved. Will need to reboot device to let webserver work...");
    isPendingReboot = true;
  });
  
  //Set the menu options
  std::vector<const char *> menu = { "wifi", "info" ,"param", "sep", "restart", "exit" };
  wifiManager.setMenu(menu);

  SerialPrintln("Attempting to connect to WiFi... Will fallback to AP mode to allow configuring of WiFi if fails...");
  if(wifiManager.autoConnect("Split-Flap-AP")) {
    SerialPrint("Successfully Connected to WiFi. IP Address: ");
    SerialPrintln(WiFi.localIP());

    isWifiConfigured = true;
  }
  
#else
  SerialPrintln("Setting up WiFi Direct");

  if (ssid != "" && password != "") {
    int maxAttemptsCount = 0;
    
    WiFi.begin(wifiDirectSsid, wifiDirectPassword);
    SerialPrint("Connecting");

    while (WiFi.status() != WL_CONNECTED && maxAttemptsCount != wifiConnectTimeoutSeconds) {
      if (maxAttemptsCount % 10 == 0) {
        SerialPrint('\n');
      }
      else {
        SerialPrint('.');
      }

      delay(1000);

      maxAttemptsCount++;      
    }

    //If we reached the max timeout
    if (maxAttemptsCount != wifiConnectTimeoutSeconds) {
      SerialPrint("Successfully Connected to WiFi. IP Address: ");
      SerialPrintln(WiFi.localIP());

      isWifiConfigured = true;
    }
  }

#endif
}
