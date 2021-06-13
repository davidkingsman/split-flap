// Comment or uncomment for testing or serial debug communication
//#define serial
//#define test

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
#define STEPS 2038 // 28BYJ-48 stepper, number of steps;
#define HALLPIN 7
#define AMOUNTFLAPS 45

bool stepperOverheated = false;
//constants others
#define BAUDRATE 9600
#define ROTATIONDIRECTION -1 //-1 for reverse direction
#define OVERHEATINGTIMEOUT 2 //timeout in seconds to avoid overheating of stepper. After starting rotation, the counter will start. Stepper won't move again until timeout is passed
unsigned long lastRotation = 0;

//globals
int displayedLetter = 0;
int desiredLetter = 0;
const String letters[] = {" ", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Ä", "Ö", "Ü", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ".", "-", "?", "!"};
Stepper stepper(STEPS, STEPPERPIN1, STEPPERPIN3, STEPPERPIN2, STEPPERPIN4); //stepper setup
bool lastInd1 = false; //store last status of phase
bool lastInd2 = false; //store last status of phase
bool lastInd3 = false; //store last status of phase
bool lastInd4 = false; //store last status of phase
float missedSteps = 0; //cummulate steps <1, to compensate via additional step when reaching >1
int currentlyrotating = 0;

int stepperSpeed = 10;

int eeAddress = 0;   //EEPROM address for calibration offset
int calOffset;       //Offset for calibration in steps

int receivedNumber = 0;

volatile unsigned long counter;
const unsigned long WAIT_TIME = 500;

//setup
void setup() {
  // i2c adress switch
  pinMode(ADRESSSW1, INPUT_PULLUP);
  pinMode(ADRESSSW2, INPUT_PULLUP);
  pinMode(ADRESSSW3, INPUT_PULLUP);
  pinMode(ADRESSSW4, INPUT_PULLUP);

  //hall sensor
  pinMode(HALLPIN, INPUT);

#ifdef serial
  //initialize serial
  Serial.begin(BAUDRATE);
  //initialize i2c
  Serial.println("starting i2c slave");
#endif

  //I2C function assignment
  Wire.begin(getaddress()); //i2c address of this unit
  Wire.onReceive(receiveLetter); //call-function if for transfered letter via i2c
  Wire.onRequest(requestEvent); //call-funtion if master requests unit state

  getOffset();      //get calibration offset from EEPROM
  calibrate(true); //home stepper
}

void loop() {
  //go to sleep and wait for instructions over i2c
  if (++counter >= WAIT_TIME)
  {
    byte old_ADCSRA = ADCSRA;
    // disable ADC
    ADCSRA = 0;
    set_sleep_mode (SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    //digitalWrite (LED_BUILTIN, LOW);
    sleep_cpu ();
    //digitalWrite (LED_BUILTIN, HIGH);
    sleep_disable();
    counter = 0;
    ADCSRA = old_ADCSRA;

    // release TWI bus
    TWCR = bit(TWEN) | bit(TWIE) | bit(TWEA) | bit(TWINT);

    // turn it back on again
    Wire.begin(getaddress());
  }  // end of time to sleep

  //check if new letter was received through i2c
  if (displayedLetter != receivedNumber)
  {
#ifdef serial
    Serial.print("Value over serial received: ");
    Serial.print(receivedNumber);
    Serial.print(" Letter: ");
    Serial.print(letters[receivedNumber]);
    Serial.println();
#endif
    //rotate to new letter
    rotateToLetter(receivedNumber);
  }


  //test calibration settings
#ifdef test
  String calLetters[10] = {" ", "Z", "A", "U", "N", "?", "0", "1", "2", "9"};
  for (int i = 0; i < 10; i++) {
    String currentCalLetter = calLetters[i];
    rotateToLetter(currentCalLetter);
    delay(5000);
  }
#endif
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
#ifdef serial
    Serial.print("go to letter: ");
    Serial.println(letters[toLetter]);
#endif
    //go to letter, but only if available (>-1)
    if (posLetter > -1) { //check if letter exists
      //check if letter is on higher index, then no full rotaion is needed
      if (posLetter >= posCurrentLetter) {
#ifdef serial
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
#ifdef serial
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
#ifdef serial
      Serial.println("letter unknown, go to space");
#endif
      desiredLetter = " ";
    }
  }
}

void receiveLetter(int numBytes) {
  counter = 0; //reset counter for sleeping

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

#ifdef serial
  Serial.print("Status ");
  Serial.print(currentlyrotating);
  Serial.print(" sent to master");
  Serial.println();
#endif

}

//returns the adress of the unit as int from 0-15
int getaddress() {
  int address = !digitalRead(ADRESSSW4) + (!digitalRead(ADRESSSW3) * 2) + (!digitalRead(ADRESSSW2) * 4) + (!digitalRead(ADRESSSW1) * 8);
  return address;
}

//gets magnet sensor offset from EEPROM in steps
void getOffset() {
  EEPROM.get(eeAddress, calOffset);
#ifdef serial
  Serial.print("CalOffset from EEPROM: ");
  Serial.print(calOffset);
  Serial.println();
#endif
}

//doing a calibration of the revolver using the hall sensor
int calibrate(bool initialCalibration) {
#ifdef serial
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
#ifdef serial
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
#ifdef serial
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
#ifdef serial
  Serial.println("Motor Stop");
#endif
  currentlyrotating = 0; //set active state to not active
  delay(100);
}

void startMotor() {
#ifdef serial
  Serial.println("Motor Start");
#endif
  currentlyrotating = 1; //set active state to active
  digitalWrite(STEPPERPIN1, lastInd1);
  digitalWrite(STEPPERPIN2, lastInd2);
  digitalWrite(STEPPERPIN3, lastInd3);
  digitalWrite(STEPPERPIN4, lastInd4);
}
