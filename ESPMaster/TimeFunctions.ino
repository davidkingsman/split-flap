void showDate() {
  timeClient.update();

  unsigned long epochTime = timeClient.getEpochTime();

  String weekDay = weekDays[timeClient.getDay()];
  //Serial.print("Week Day: ");
  //Serial.println(weekDay);

  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime);

  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon + 1;
  int currentYear = ptm->tm_year + 1900;

  String currentMonthName = months[currentMonth - 1];

  String currentDate;

  //Add leading zeroes
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

  //Add leading zero
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
  //Needed for time functions
  timeClient.begin();
  timeClient.setTimeOffset(TIMEOFFSET);
  #ifdef serial
  Serial.println("timeClient initialized");
#endif
}
