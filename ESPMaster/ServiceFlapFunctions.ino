//Shows a new message on the display
void showText(String message) {  
  showText(message, 0);
}

void showText(String message, int delayMillis) {  
  if (lastWrittenText != message || alignmentUpdated) { 
    String messageDisplay = message == "" ? "<Blank>" : message;
    String alignmentUpdatedDisplay = alignmentUpdated ? "Yes" : "No";

    SerialPrintln("Showing new Message");
    SerialPrintln("New Message: " + messageDisplay);
    SerialPrintln("Alignment Updated: " + alignmentUpdatedDisplay);
  
    LList<String> messageLines = processSentenceToLines(message);

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
      SerialPrintln("Showing a simple message: " + message);
      
      showMessage(message, convertSpeed(flapSpeed));
    }  

    //If the device wasn't previously in text mode, delay for a short time so can read!
    if (delayMillis != 0) {
      SerialPrintln("Pausing for a small duration. Delay: " + String(delayMillis));
      delay(delayMillis);
    }

    //Save what we last did
    lastWrittenText = message;

    //Alignment definitely has not changed now
    alignmentUpdated = false;
    
    SerialPrintln("Done showing message");
  }
}

//Pushes message to units
void showMessage(String message, int flapSpeed) {
  //Format string per alignment choice
  if (alignment == ALIGNMENT_MODE_LEFT) {
    message = leftString(message);
  } 
  else if (alignment == ALIGNMENT_MODE_RIGHT) {
    message = rightString(message);
  } 
  else if (alignment == ALIGNMENT_MODE_CENTER) {
    message = centerString(message);
  }

  SerialPrint("Showing Aligned Message: \"");
  SerialPrint(message);
  SerialPrintln("\"");

#if UNIT_CALLS_DISABLE == true
  SerialPrintln("Unit Calls are disabled for debugging. Will delay to simulate calls...");
  delay(2000);
#else
  //Wait while display is still moving
  SerialPrintln("Unit calls are enabled. Will display message");
  while (isDisplayMoving()) {
    SerialPrintln("Waiting for display to stop");
    delay(500);
  }

  for (int unitIndex = 0; unitIndex < UNITS_AMOUNT; unitIndex++) {
    char currentLetter = message[unitIndex];
    int currentLetterPosition = translateLettertoInt(currentLetter);
    
    SerialPrint("Unit Nr.: ");
    SerialPrint(unitIndex);
    SerialPrint(" Letter: ");
    SerialPrint(message[unitIndex]);
    SerialPrint(" Letter position: ");
    SerialPrintln(currentLetterPosition);

    //only write to unit if char exists in letter array
    if (currentLetterPosition != -1) {
      writeToUnit(unitIndex, currentLetterPosition, flapSpeed);
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
  for (int flapIndex = 0; flapIndex < FLAP_AMOUNT; flapIndex++) {
    if (letterchar == letters[flapIndex]) {
      return flapIndex;
    }
  }

  return -1;
}

//Write letter position and speed in rpm to single unit
void writeToUnit(int address, int letter, int flapSpeed) {
  int sendArray[2] = {letter, flapSpeed}; //Array with values to send to unit

  Wire.beginTransmission(address);

  //Write values to send to slave in buffer
  for (unsigned int index = 0; index < sizeof sendArray / sizeof sendArray[0]; index++) {
    SerialPrint("sendArray: ");
    SerialPrintln(sendArray[index]);

    Wire.write(sendArray[index]);
  }
  Wire.endTransmission(); //send values to unit
}

//Checks if unit in display is currently moving
bool isDisplayMoving() {
  //Request all units moving state and write to array
  for (int unitIndex = 0; unitIndex < UNITS_AMOUNT; unitIndex++) {
    displayState[unitIndex] = checkIfMoving(unitIndex);
    if (displayState[unitIndex] == 1) {
      SerialPrintln("A unit in the display is busy");
      return true;
    } 
    //If unit is not available through i2c
    else if (displayState[unitIndex] == -1) {
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
  Wire.requestFrom(address, ANSWER_SIZE, 1);
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
