//Show the current date on the display
void showDate() {
  showText(timezone.dateTime(dateFormat));
}

//Show the current time on the display
void showClock() {
  showText(timezone.dateTime(clockFormat));
}
