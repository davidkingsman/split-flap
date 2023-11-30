/*********
  Split Flap Arduino Nano Unit
*********/

//#define SERIAL_ENABLE // uncomment for serial debug communication
//#define TEST_ENABLE   // uncomment for Test mode. Rotates through a few character to make sure unit is working. These characters should be displayed in the correct order: " ", "Z", "A", "U", "N", "?", "0", "1", "2", "9"

#include <Arduino.h>
#include <Wire.h>
#include <Stepper.h>
#include <EEPROM.h>
#include <avr/sleep.h>

// Pins of I2C adress switch
#define ADRESSSW1 6
#define ADRESSSW2 5
#define ADRESSSW3 4
#define ADRESSSW4 3

//constants stepper
#define STEPPERPIN1 11
#define STEPPERPIN2 10
#define STEPPERPIN3 9
#define STEPPERPIN4 8
#define STEPS 2038 //28BYJ-48 stepper, number of steps
#define HALLPIN 7 //Pin of hall sensor
#define AMOUNTFLAPS 45

//constants others
#define BAUDRATE 115200
#define ROTATIONDIRECTION -1 //-1 for reverse direction
#define OVERHEATINGTIMEOUT 2 //timeout in seconds to avoid overheating of stepper. After starting rotation, the counter will start. Stepper won't move again until timeout is passed
unsigned long lastRotation = 0;

//globals
int displayedLetter = 0; //currently shown letter
int desiredLetter = 0; //letter to be shown
const String letters[] = {" ", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Ä", "Ö", "Ü", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ".", "-", "?", "!"};
Stepper stepper(STEPS, STEPPERPIN1, STEPPERPIN3, STEPPERPIN2, STEPPERPIN4); //stepper setup
bool lastInd1 = false; //store last status of phase
bool lastInd2 = false; //store last status of phase
bool lastInd3 = false; //store last status of phase
bool lastInd4 = false; //store last status of phase
float missedSteps = 0; //cummulate steps <1, to compensate via additional step when reaching >1
int currentlyrotating = 0; // 1 = drum is currently rotating, 0 = drum is standing still
int stepperSpeed = 10; //current speed of stepper, value only for first homing
int eeAddress = 0;   //EEPROM address for calibration offset
int calOffset;       //Offset for calibration in steps, stored in EEPROM, gets read in setup
int receivedNumber = 0;
int i2cAddress;

//sleep globals
const unsigned long WAIT_TIME = 2000;    //wait time before sleep routine gets executed again in milliseconds
unsigned long previousMillis = 0;       //stores last time sleep was interrupted

//setup
void setup() {
  // i2c adress switch
  pinMode(ADRESSSW1, INPUT_PULLUP);
  pinMode(ADRESSSW2, INPUT_PULLUP);
  pinMode(ADRESSSW3, INPUT_PULLUP);
  pinMode(ADRESSSW4, INPUT_PULLUP);

  //hall sensor
  pinMode(HALLPIN, INPUT);

  i2cAddress = getaddress(); //get I2C Address and save in variable

#ifdef SERIAL_ENABLE
  //initialize serial
  Serial.begin(BAUDRATE);
  Serial.println("starting unit");
  Serial.print("I2CAddress: ");
  Serial.println(i2cAddress);
#endif

  //I2C function assignment
  Wire.begin(i2cAddress); //i2c address of this unit
  Wire.onReceive(receiveLetter);//call-function for transfered letter via i2c
  Wire.onRequest(requestEvent); //call-funtion if master requests unit state

  getOffset();     //get calibration offset from EEPROM
  calibrate(true); //home stepper after startup

  //test calibration settings
#ifdef TEST_ENABLE
  int calLetters[10] = {0, 26, 1, 21, 14, 43, 30, 31, 32, 39};
  for (int i = 0; i < 10; i++) {
    int currentCalLetter = calLetters[i];
    rotateToLetter(currentCalLetter);
    delay(5000);
  }
#endif
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= WAIT_TIME) {
    byte old_ADCSRA = ADCSRA;
    // disable ADC
    ADCSRA = 0;
    set_sleep_mode (SLEEP_MODE_PWR_DOWN);
    sleep_enable();
#ifdef SERIAL_ENABLE
    digitalWrite (LED_BUILTIN, LOW); // shuts off LED when starting to sleep, for debugging
#endif
    sleep_cpu ();
#ifdef SERIAL_ENABLE
    digitalWrite (LED_BUILTIN, HIGH); // turns on LED when waking up, for debugging
#endif
    sleep_disable();
    previousMillis = currentMillis; //reset sleep counter
    ADCSRA = old_ADCSRA;

    // release TWI bus
    TWCR = bit(TWEN) | bit(TWIE) | bit(TWEA) | bit(TWINT);

    // turn it back on again
    Wire.begin (i2cAddress);
  }  // end of time to sleep

  //check if new letter was received through i2c
  if (displayedLetter != receivedNumber)
  {
    /*
      #ifdef SERIAL_ENABLE
      Serial.print("Value over serial received: ");
      Serial.print(receivedNumber);
      Serial.print(" Letter: ");
      Serial.print(letters[receivedNumber]);
      Serial.println();
      #endif
    */
    //rotate to new letter
    rotateToLetter(receivedNumber);
  }
}

//rotate to letter
void rotateToLetter(int toLetter) {
  if (lastRotation == 0 || (millis() - lastRotation > OVERHEATINGTIMEOUT * 1000)) {
    lastRotation = millis();
    //get letter position
    int posLetter = -1;
    posLetter = toLetter;
    int posCurrentLetter = -1;
    posCurrentLetter = displayedLetter;
    //int amountLetters = sizeof(letters) / sizeof(String);
#ifdef SERIAL_ENABLE
    Serial.print("go to letter: ");
    Serial.println(letters[toLetter]);
#endif
    //go to letter, but only if available (>-1)
    if (posLetter > -1) { //check if letter exists
      //check if letter is on higher index, then no full rotaion is needed
      if (posLetter >= posCurrentLetter) {
#ifdef SERIAL_ENABLE
        Serial.println("direct");
#endif
        //go directly to next letter, get steps from current letter to target letter
        int diffPosition = posLetter - posCurrentLetter;
        startMotor();
        stepper.setSpeed(stepperSpeed);
        //doing the rotation letterwise
        for (int i = 0; i < diffPosition; i++) {
          float preciseStep = (float)STEPS / (float)AMOUNTFLAPS;
          int roundedStep = (int)preciseStep;
          missedSteps = missedSteps + ((float)preciseStep - (float)roundedStep);
          if (missedSteps > 1) {
            roundedStep = roundedStep + 1;
            missedSteps--;
          }
          stepper.step(ROTATIONDIRECTION * roundedStep);
        }
      }
      else {
        //full rotation is needed, good time for a calibration
#ifdef SERIAL_ENABLE
        Serial.println("full rotation incl. calibration");
#endif
        calibrate(false); //calibrate revolver and do not stop motor
        //startMotor();
        stepper.setSpeed(stepperSpeed);
        for (int i = 0; i < posLetter; i++) {
          float preciseStep = (float)STEPS / (float)AMOUNTFLAPS;
          int roundedStep = (int)preciseStep;
          missedSteps = missedSteps + (float)preciseStep - (float)roundedStep;
          if (missedSteps > 1) {
            roundedStep = roundedStep + 1;
            missedSteps--;
          }
          stepper.step(ROTATIONDIRECTION * roundedStep);
        }
      }
      //store new position
      displayedLetter = toLetter;
      //rotation is done, stop the motor
      delay(100); //important to stop rotation before shutting of the motor to avoid rotation after switching off current
      stopMotor();
    }
    else {
#ifdef SERIAL_ENABLE
      Serial.println("letter unknown, go to space");
#endif
      desiredLetter = 0;
    }
  }
}

void receiveLetter(int numBytes) {
  int receiveArray[2]; //array for received bytes

  for (int i = 0; i < numBytes; i++) {
    receiveArray[i] = Wire.read();
  }
  //Write received bytes to correct variables
  receivedNumber = receiveArray[0];
  stepperSpeed = receiveArray[1];
}

void requestEvent() {
  Wire.write(currentlyrotating); //send unit status to master
  /*
    #ifdef SERIAL_ENABLE
    Serial.print("Status ");
    Serial.print(currentlyrotating);
    Serial.print(" sent to master");
    Serial.println();
    #endif
  */
}

//returns the adress of the unit as int from 0-15
int getaddress() {
  int address = !digitalRead(ADRESSSW4) + (!digitalRead(ADRESSSW3) * 2) + (!digitalRead(ADRESSSW2) * 4) + (!digitalRead(ADRESSSW1) * 8);
  return address;
}

//gets magnet sensor offset from EEPROM in steps
void getOffset() {
  EEPROM.get(eeAddress, calOffset);
#ifdef SERIAL_ENABLE
  Serial.print("CalOffset from EEPROM: ");
  Serial.print(calOffset);
  Serial.println();
#endif
}

//doing a calibration of the revolver using the hall sensor
int calibrate(bool initialCalibration) {
#ifdef SERIAL_ENABLE
  Serial.println("calibrate revolver");
#endif
  currentlyrotating = 1; //set active state to active
  bool reachedMarker = false;
  stepper.setSpeed(stepperSpeed);
  int i = 0;
  while (!reachedMarker) {
    int currentHallValue = digitalRead(HALLPIN);
    if (currentHallValue == 1 && i == 0) { //already in zero position move out a bit and do the calibration {
      //not reached yet
      i = 50;
      stepper.step(ROTATIONDIRECTION * 50); //move 50 steps to get out of scope of hall
    }
    else if (currentHallValue == 1) {
      //not reached yet
      stepper.step(ROTATIONDIRECTION * 1);
    }
    else {
      //reached marker, go to calibrated offset position
      reachedMarker = true;
      stepper.step(ROTATIONDIRECTION * calOffset);
      displayedLetter = 0;
      missedSteps = 0;
#ifdef SERIAL_ENABLE
      Serial.println("revolver calibrated");
#endif
      //Only stop motor for initial calibration
      if (initialCalibration) {
        stopMotor();
      }
      return i;
    }
    if (i > 3 * STEPS) {
      //seems that there is a problem with the marker or the sensor. turn of the motor to avoid overheating.
      displayedLetter = 0;
      desiredLetter = 0;
      reachedMarker = true;
#ifdef SERIAL_ENABLE
      Serial.println("calibration revolver failed");
#endif
      stopMotor();
      return -1;
    }
    i++;
  }
  return i;
}

//switching off the motor driver
void stopMotor() {
  lastInd1 = digitalRead(STEPPERPIN1);
  lastInd2 = digitalRead(STEPPERPIN2);
  lastInd3 = digitalRead(STEPPERPIN3);
  lastInd4 = digitalRead(STEPPERPIN4);

  digitalWrite(STEPPERPIN1, LOW);
  digitalWrite(STEPPERPIN2, LOW);
  digitalWrite(STEPPERPIN3, LOW);
  digitalWrite(STEPPERPIN4, LOW);
#ifdef SERIAL_ENABLE
  Serial.println("Motor Stop");
#endif
  currentlyrotating = 0; //set active state to not active
  delay(100);
}

void startMotor() {
#ifdef SERIAL_ENABLE
  Serial.println("Motor Start");
#endif
  currentlyrotating = 1; //set active state to active
  digitalWrite(STEPPERPIN1, lastInd1);
  digitalWrite(STEPPERPIN2, lastInd2);
  digitalWrite(STEPPERPIN3, lastInd3);
  digitalWrite(STEPPERPIN4, lastInd4);
}
