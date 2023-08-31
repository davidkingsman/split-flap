//aligns string center by filling the left and right with spaces up the max of the units amount
String centerString(String message) {
  //Takes care of the left side of the text (if any)
  int leftSpaceAmount = (UNITSAMOUNT -  message.length()) / 2;
  for (int i = 0; i < leftSpaceAmount; i++) {
    message = " " + message;
  }

  //Take care of the right side of the text (if any)
  for(int i = message.length(); i < UNITSAMOUNT; i++) {
    message = message + " ";
  }

  message = cleanString(message);
  
  return message;
}

//aligns string on right side of array and fills empty chars with spaces
String rightString(String message) {
  int rightSpaceAmount = (UNITSAMOUNT - message.length());
  for (int i = 0; i < rightSpaceAmount; i++) {
    message = " " + message;
  }

  message = cleanString(message);

  return message;
}

//aligns string on left side of array and fills empty chars with spaces
String leftString(String message) {
  int leftSpaceAmount = (UNITSAMOUNT - message.length());
  for (int i = 0; i < leftSpaceAmount; i++) {
    message = message + " ";
  }

  message = cleanString(message);

  return message;
}

//converts input string to uppercase
String cleanString(String message) {
  message.toUpperCase();
  return message;
}

int convertSpeed(String speedSlider) {
  int speedSliderInt;
  speedSliderInt = speedSlider.toInt();
  speedSliderInt = map(speedSliderInt, 1, 100, MINSPEED, MAXSPEED);
  return speedSliderInt;
}
