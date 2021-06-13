//checks for new message to show
void showNewData(String message) {
#ifndef serial
  if (writtenLast != message) {
    showMessage(message, convertSpeed(speedslider));
  }
#endif

#ifdef serial
  if (writtenLast != message) {
    Serial.println("Would be sent to display");
    Serial.println(centerString(message));
  }
#endif
  writtenLast = message;
}

//pushes message to units
void showMessage(String message, int flapSpeed) {

  //Format string per alignment choice
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
    Serial.println(message[i]);
    Serial.println(currentLetterPosition);
#endif
    writeToUnit(i, currentLetterPosition, flapSpeed);
  }
}

//translates char to letter position
int translateLettertoInt(char letterchar) {
  for (int i = 0; i < FLAPAMOUNT; i++) {
    if (letterchar == letters[i]) {
      return i;
    }
  }
}

//write letter position and speed in rpm to single unit
void writeToUnit(int address, int letter, int flapSpeed) {
  int sendArray[2] = {letter, flapSpeed}; //Array with values to send to unit
  Wire.beginTransmission(address);

  //Write values to send to slave in buffer
  for (int i = 0; i < sizeof sendArray / sizeof sendArray[0]; i++) {
    Wire.write(sendArray[i]);
  }
  Wire.endTransmission(); //send values to unit
}

//checks if unit in display is currently moving
bool isDisplayMoving() {
  //Request all units moving state and write to array
  for (int i = 0; i < UNITSAMOUNT; i++) {
    displayState[i] = checkIfMoving(i);
    if (displayState[i] == 1) {
#ifdef serial
      Serial.println("A unit in the display is busy");
#endif
      return true;

      //if unit is not available through i2c
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

//checks if single unit is moving
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
    Serial.println("Try to wake up unit");
#ifdef serial
    Wire.beginTransmission(address);
    Wire.endTransmission();
#endif
    delay(5);
  }
  return active;
}
