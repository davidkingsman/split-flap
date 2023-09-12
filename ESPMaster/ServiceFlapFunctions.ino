//Shows a new message on the display
void showText(String message) {  
  if (lastWrittenText != message) { 
    SerialPrintln("Showing new Message");
    SerialPrintln("Last Written Text: " + lastWrittenText);
    SerialPrintln("New Message: " + message);
  
    LinkedList<String> messageLines = processSentenceToLines(message);

    if (messageLines.size() > 1) {
      SerialPrintln("Showing a split down message");
    
      //Iterate over all the message lines we've got
      for (int linesIndex = 0; linesIndex < messageLines.size(); linesIndex++) {
        String line = messageLines[linesIndex];

        SerialPrint("-- Message Line: ");
        SerialPrintln(line);

        showMessage(line, convertSpeed(flapSpeed));
    
        //If the lines index isn't the last, delay showing the next message to give time to read
        if (linesIndex <= messageLines.size()) {
          delay(3000);
        }
      }  
    }
    else {
      SerialPrint("Showing a simple message: ");
      SerialPrintln(message);
  
      showMessage(message, convertSpeed(flapSpeed));
    }  

    //If the device wasn't previously in text mode, delay for a short time so can read!
    if (currentDeviceMode != previousDeviceMode) {
      delay(3000);
      currentDeviceMode = previousDeviceMode;
    }

    //Save what we last did
    lastWrittenText = message;
    
    SerialPrintln("Done showing message");
  }
}

//Pushes message to units
void showMessage(String message, int flapSpeed) {
  SerialPrint("-- Show Message Received for: \"");
  SerialPrint(message);
  SerialPrintln("\"");

  //Format string per alignment choice
  if (alignment == ALIGNMENT_MODE_LEFT) {
    message = leftString(message);
  } else if (alignment == ALIGNMENT_MODE_RIGHT) {
    message = rightString(message);
  } else if (alignment == ALIGNMENT_MODE_CENTER) {
    message = centerString(message);
  }

  SerialPrint("-- Aligned Message: \"");
  SerialPrint(message);
  SerialPrintln("\"");

#ifdef UNIT_CALLS_DISABLE
  SerialPrintln("Unit Calls are disabled for debugging. Will delay to simulate calls...");
  delay(2000);
#endif

#ifndef UNIT_CALLS_DISABLE
  //Wait while display is still moving
  while (isDisplayMoving()) {
    SerialPrintln("Waiting for display to stop");
    delay(500);
  }

  SerialPrintln(message);
  for (int i = 0; i < UNITSAMOUNT; i++) {
    char currentLetter = message[i];
    int currentLetterPosition = translateLettertoInt(currentLetter);
    
    SerialPrint("Unit Nr.: ");
    SerialPrint(i);
    SerialPrint(" Letter: ");
    SerialPrint(message[i]);
    SerialPrint(" Letter position: ");
    SerialPrintln(currentLetterPosition);

    //only write to unit if char exists in letter array
    if (currentLetterPosition != -1) {
      writeToUnit(i, currentLetterPosition, flapSpeed);
    }
  }

  //Wait for the display to stop moving before exit
  while (isDisplayMoving()) {
    SerialPrintln("Waiting for display to stop now message is display");
    delay(100);
  }
#endif
}

//Translates char to letter position
int translateLettertoInt(char letterchar) {
  for (int i = 0; i < FLAPAMOUNT; i++) {
    if (letterchar == letters[i]) {
      return i;
    }
  }
  return -1;
}

//Write letter position and speed in rpm to single unit
void writeToUnit(int address, int letter, int flapSpeed) {
  int sendArray[2] = {letter, flapSpeed}; //Array with values to send to unit

  Wire.beginTransmission(address);

  //Write values to send to slave in buffer
  for (unsigned int i = 0; i < sizeof sendArray / sizeof sendArray[0]; i++) {
    SerialPrint("sendArray: ");
    SerialPrintln(sendArray[i]);

    Wire.write(sendArray[i]);
  }
  Wire.endTransmission(); //send values to unit
}

//Checks if unit in display is currently moving
bool isDisplayMoving() {
  //Request all units moving state and write to array
  for (int i = 0; i < UNITSAMOUNT; i++) {
    displayState[i] = checkIfMoving(i);
    if (displayState[i] == 1) {
      SerialPrintln("A unit in the display is busy");
      return true;
    } 
    //If unit is not available through i2c
    else if (displayState[i] == -1) {
      SerialPrintln("A unit in the display is sleeping");
      return true;
    }
  }

  SerialPrintln("Display is standing still");
  return false;
}

//Checks if single unit is moving
int checkIfMoving(int address) {
  int active;
  Wire.requestFrom(address, ANSWERSIZE, 1);
  active = Wire.read();

  SerialPrint(address);
  SerialPrint(":");
  SerialPrintln(active);

  if (active == -1) {
    SerialPrintln("Try to wake up unit");
    Wire.beginTransmission(address);
    Wire.endTransmission();
  }
  
  return active;
}
