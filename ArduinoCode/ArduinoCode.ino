/*
<<<File Info>>>
Description: Starter code for a project

Developer: Jonathan Brunath

Date Created: 4/8/10/2015
Updated: 7/11/12/2016
Updated: 1/12/12/2016
*/

#include "Arduino.h"
#include "tables.h"

//
//Variable Definitions
//

bool debugEnabled = false; //toggles debugging output over serial communications

//constants (values that can not be changed while the system is running)
const int numberOfMotors = 1; //defines the number of motors in the system
const int numberOfLEDs = 1; //defines the number of LED's in the system
const int numberOfPIRs = 1; //defines the number of PIR's in the system

//pinout
int motorPins[numberOfMotors]; //array to store motor I/O pin numbers
int LEDPins[numberOfLEDs]; //array to store led I/O pin numbers
int PIRPins[numberOfLEDs]; //array to store PIR I/O pin numbers
int defaultLEDPins[] = {4}; //defines the default LED pin numbers (number of pins can differ from value of "numberOfLEDs", extras will be ignored during configuration)
int defaultMotorPins[] = {3}; //defines the default Motor pin numbers (number of pins can differ from value of "numberOfMotors", extras will be ignored during configuration)
int defaultPIRPins[] = {7}; //defines the default PIR pin numbers (number of pins can differ from value of "numberOfPIRs", extras will be ignored during configuration)

//general variables
bool motorState[numberOfMotors]; //array to store current state of the motors
bool LEDState[numberOfLEDs]; //array to store current state of the LED's
int motorSpeed[numberOfMotors]; //array to store the speed of each motor
bool pirOutput[numberOfPIRs]; //array to store the output of each PIR sensor
bool defaultLEDState = true; //defines the default state for the LED's
bool defaultMotorState = false; //defines the default state for the motors
int maxRPM[numberOfMotors] = {10000}; //defines the maximum RPM of each of the motors
int defaultUpperTargetMotorSpeed = maxRPM[0]; //defines the default upper target speed of the motors
int defaultLowerTargetMotorSpeed = 0; //defines the default lower target speed of the motors
int runMode = 2; //defines which mode to run the system in
int maxDelay = 1000; //defines the maximum delay in milliseconds between increments of the motor speed ramping function
int defaultRampDelay = 0; //defines the default delay in milliseconds between increments for ramping the speed of a motor
int defaultRampIncrement = 500; //defines the default ramping increment for ramping the speed of a motor
int rampIncrement[numberOfMotors]; //stores the ramping increment value for each motor;

//command processing variables
String cmdS; //String to store incoming command
String valS; //String to store first command parameter
String val2S; //String to store first command parameter
char seperator = ','; //character to use to differentiate between the command and parameters in an incoming serial String (command)
char delimiter = '\n'; //character to use to signify the end of an incoming serial String (command)
bool cmdsEnabled = true; //boolean to store weather commands are enabled and will be processed or not
int baudRate = 9600; //integer to store the baud rate to use for serial communications (similar to transfer speed)

//menu variables
int consoleWidth = 25; //defines the default number of characters in the width of serial communications output console
int consoleHeight = 25; //defines the default number of characters in the height of serial communications output console
char lineBreakCharacter = '-'; //defines the default character to print when printing a line break to denote different sections of a texted based menu over serial communications
int currentMenu = 0; //denotes which menu is currently selected
char refreshMenuChar='r';
char returnToLastMenuChar='b';

//
//Core System Functions
//

//initialize system settings and start basic processes
void setup() {
  serialControl(); //enable serial port if commands are enabled

  //configure motors
  //update motorPins array with default pin values and set new pins numbers as outputs
  for (int i = 0; i < numberOfMotors; i++) {
    configureMotor(i, defaultMotorPins[i]);
  }

  //configure LED's
  //update LEDPins array with default pin values and set new pins numbers as outputs
  for (int i = 0; i < numberOfLEDs; i++) {
    configureLED(i, defaultLEDPins[i]);
  }

  //configure PIR's
  //update PIRPins array with default pin values and set new pins numbers as inputs
  for (int i = 0; i < numberOfPIRs; i++) {

    debug("Default PIR Pin: "+defaultPIRPins[i]);
    configurePIR(i, defaultPIRPins[i]);
  }
}

//repeat the following code block for each cycle of the cpu
void loop() {
  //process incoming commands from serial port if enabled
  if (cmdsEnabled) {
    processCmds();
  }
  debug("PIR State: "+getPIRState(0));
  debug("");
  //delay(100);
  //determine which mode to run the system in
  if (runMode == 1) {
    //run system using basic on/off control for the motors
    setMotorState(0, pirOutput[0]); //set the motor state to the output of the PIR sensor
  } else if (runMode == 2) {
    //run system using ramp up/down speeds for motors
    //check if the PIR output has changed since the last time it was checked
    if (pirOutput[0]) {
      rampMotorSpeedUp(0); //ramp the motor speed
    } else {
      rampMotorSpeedDown(0); //ramp the motor speed
    }
  }
}

//
//I/O Control Functions
//

//toggle digital pin on or off
void toggleDigialOutput(int pinNumber, bool *storedState) {
  //set pin "pinNumber" state to the inverse of the on current pin state as specified by the stored state value "storedState"
  setDigitalOutput(pinNumber, !*storedState, storedState);
}
//set digital pin on or off
void setDigitalOutput(int pinNumber, bool state, bool *storedState) {
  //determine weather to turn pin "pinNumber" on or off
  if (state) {
    digitalWrite(pinNumber, HIGH); //set pin "pinNumber" to ON
  } else {
    digitalWrite(pinNumber, LOW); //set pin "pinNumber" to OFF
  }
  //set stored state variable
  *storedState = state;
}
//set analog pin on or off
void setAnalogOutput(int pinNumber, int output, int *storedOutput) {
  //set the analog output to the specified value
  analogWrite(pinNumber, output);

  //set stored state variable
  *storedOutput = output;
}
//configure new digital output pin
void configureDigitalOutputPin(int pinNumber, bool *storedState, bool defaultState) {
  //set digital pin "pinNumber" as an output
  pinMode(pinNumber, OUTPUT);

  //set new pin number to OFF state and update the pins corosponding stored state variable
  setDigitalOutput(pinNumber, defaultState, storedState);
}
//configure new digital input pin
void configureDigitalInputPin(int pinNumber, bool *storedState) {
  //set digital pin "pinNumber" as an input
  pinMode(pinNumber, INPUT);

  //read the initial input value and store it in the storedState variable
  *storedState = digitalRead(pinNumber);
}

//
//Passive Infrared Sensor (PIR) Control Functions
//

//Configure PIR pin
void configurePIR(int pirNumber, int pinNumber) {
  //check if specified pirNumber is valid
  if ((pirNumber >= 0) && (pirNumber < numberOfPIRs)) {
    //set "pirPin" array index "pirID" to equal new pin number
    PIRPins[pirNumber] = pinNumber;

    //pass PIR pin number and corosponding stored output variable to "configureDigitalInputPin"
    configureDigitalInputPin(PIRPins[pirNumber], &pirOutput[pirNumber]);
  }
}
//get specific PIR value
bool getPIRState(int pirNumber) {
  //check if specified pirNumber is valid
  if ((pirNumber >= 0) && (pirNumber < numberOfPIRs)) {
    //return the value new output of the specified PIR and update the stored output
    pirOutput[pirNumber] = digitalRead(PIRPins[pirNumber]);
    return pirOutput[pirNumber];
  }

  //return false if specified pirNumber is out of range
  return false;
}

//
//Motor Control Functions
//

//Configure specific motor pin
void configureMotor(int motorNumber, int pinNumber) {
  //check if specified pirNumber is valid
  if ((motorNumber >= 0) && (motorNumber < numberOfMotors)) {
    //set "motorPins" array index "motorNumber" to equal "pinNumber"
    motorPins[motorNumber] = pinNumber;

    //pass motor pin number, stored state variable, and inital state to "configureDigitalOutputPin"
    configureDigitalOutputPin(motorPins[motorNumber], &motorState[motorNumber], defaultMotorState);

    //Set default Motor initial motor speed to zero
    setAnalogOutput(motorPins[motorNumber], 0, &motorSpeed[motorNumber]);
  }
}
//Set default Motor on or off (speed=0 or speed=max)
void setMotorState(int motorNumber, bool state) {
  //check if specified motorNumber is valid
  if ((motorNumber >= 0) && (motorNumber < numberOfMotors)) {
    //pass specified motor pin number and stored motor state along with the new state to "setOuput" function
    setDigitalOutput(motorPins[motorNumber], state, &motorState[motorNumber]);
  }
}
//convert an RPM value to analog output range
int rpmToAnalog(int motorNumber, int rpm) {
  //check if specified motorNumber is valid
  if ((motorNumber >= 0) && (motorNumber < numberOfMotors)) {
    //map the RPM range of the specified motor to the range of the analog output
    return map(rpm, 0, maxRPM[motorNumber], 0, 255);
  }

  //return a value of -1 to signify that the specified motor number was invalid
  return -1;
}
//set the speed of a specific motor
void setMotorSpeed(int motorNumber, int speed) {
  //check if specified motorNumber is valid
  if ((motorNumber >= 0) && (motorNumber < numberOfMotors)) {
    //check if specified speed is valid
    if ((speed >= 0) && (speed < maxRPM[motorNumber])) {
      //pass specified motor pin number and stored motor speed along with the new state to "setOuput" function
      setAnalogOutput(motorPins[motorNumber], rpmToAnalog(motorNumber, speed), &motorSpeed[motorNumber]);
    }
  }
}
//toggle motor state
void toggleMotorState(int motorNumber) {
  //check if specified motorNumber is valid
  if ((motorNumber >= 0) && (motorNumber < numberOfMotors)) {
    //pass specified motor pin number and stored motor state to "toggleOuput" function
    toggleDigialOutput(motorPins[motorNumber], &motorState[motorNumber]);
  }
}
//set a specific motor's maximum RPM
void setMaxRPM(int motorNumber, int max) {
  //check if specified motorNumber is valid
  if ((motorNumber >= 0) && (motorNumber < numberOfMotors)) {
    maxRPM[motorNumber] = max;
  }
}
//change a motors speed gradually over time
void rampMotorSpeed(int motorNumber, int targetSpeed, int increment, int delayTime) {
  //check if specified motorNumber is valid
  if ((motorNumber >= 0) && (motorNumber < numberOfMotors)) {
      debug();
      debug("Current Speed: "+motorSpeed[motorNumber]);
      debug("Target Speed: "+targetSpeed);
      debug("Increment: "+increment);
      debug("Delay: "+delayTime);

    //determine weather the speed needs to ramp up or down
    if (motorSpeed[motorNumber] < targetSpeed) {
        debug();
        debug("ramp up");
        debug();
        //delay(1000);
      }
      //start at initial motor speed, incrementing up by the specified value until the targetSpeed is reached
      for (int i = motorSpeed[motorNumber]; i <= targetSpeed; i += increment) {
          debug("RPM: "+i);
          debug("Analog Output: "+rpmToAnalog(motorNumber, i));
          debug();
        setAnalogOutput(motorPins[motorNumber], rpmToAnalog(motorNumber, i), &motorSpeed[motorNumber]);
        if ((delayTime > 1) && (delayTime < maxDelay)) {
          delay(delayTime);
        }
      }
    } else if (motorSpeed[motorNumber] > targetSpeed) {
        debug();
        debug("ramp down");
        debug();
        //delay(1000);
      //start at initial motor speed, incrementing down by the specified value until the targetSpeed is reached
      for (int i = motorSpeed[motorNumber]; i >= targetSpeed; i -= increment) {
        debug("RPM: "+i);
        debug("Analog Output: "+rpmToAnalog(motorNumber, i));
        debug();
        setAnalogOutput(motorPins[motorNumber], rpmToAnalog(motorNumber, i), &motorSpeed[motorNumber]);

        if ((delayTime > 1) && (delayTime < maxDelay)) {
          delay(delayTime);
        }
      }
    }

    motorSpeed[motorNumber] = targetSpeed;
  }
}
//call rampMotorSpeed function using the default increment and delay values
void rampMotorSpeed(int motorNumber, int targetSpeed) {
  rampMotorSpeed(motorNumber, targetSpeed, defaultRampIncrement, defaultRampDelay);
}
//call rampMotorSpeed function using the default upper target motor speed, increment and delay values
void rampMotorSpeedUp(int motorNumber) {
  rampMotorSpeed(motorNumber, defaultUpperTargetMotorSpeed, defaultRampIncrement, defaultRampDelay);
}
//call rampMotorSpeed function using the default lower target motor speed, increment and delay values
void rampMotorSpeedDown(int motorNumber) {
  rampMotorSpeed(motorNumber, defaultLowerTargetMotorSpeed, defaultRampIncrement, defaultRampDelay);
}

//
//LED Control Functions
//

//Configure specific LED pin
void configureLED(int LEDNumber, int pinNumber) {
  //check if specified LEDNumber is valid
  if ((LEDNumber >= 0) && (LEDNumber < numberOfLEDs)) {
    //set "LEDPins" array index "LEDNumber" to equal "pinNumber"
    LEDPins[LEDNumber] = pinNumber;

    //pass LED pin number, stored state variable, and the initial LED state to "configureDigitalOutputPin" function
    configureDigitalOutputPin(LEDPins[LEDNumber], &LEDState[LEDNumber], defaultLEDState);
  }
}
//set LED on or off
void setLEDState(int LEDNumber, bool state) {
  //pass specified LED pin number and stored LED state along with the new state to "setOuput" function
  setDigitalOutput(LEDPins[LEDNumber], state, &LEDState[LEDNumber]);
}
//toggle LED state
void toggleLEDState(int LEDNumber) {
  //pass specified LED pin number and stored LED state to "toggleOuput" function
  toggleDigialOutput(LEDPins[LEDNumber], &LEDState[LEDNumber]);
}

//
//Serial Control functions
//

//Start or stop serial port
void serialControl(bool state) {
  if (state) {
    Serial.begin(baudRate);
  } else {
    Serial.end();
  }
}
//Start or stop serial port if commands are enabled
void serialControl() {
  serialControl(cmdsEnabled);
}

//
//Command Processing
//

//process commands from the serial port
void processCmds() {
  /*
  NOTICE: the way commands are current parsed can be made much more efficent. The
  code will be updated if the need arises and time allows for it.
  */

  //read incoming command with parameters if serial port is available(active)
  while (Serial.available()) {
    cmdS = Serial.readStringUntil(seperator); //read command
    valS = Serial.readStringUntil(seperator); //read first command parameter
    val2S = Serial.readStringUntil(delimiter); //read second command parameter
  }

  switch (currentMenu) {
    case 0:
      mainMenu();
      break;
    case 1:
      MotorMenu();
      break;
    case 2:
      LEDMenu();
      break;
    case 3:
      PIRMenu();
      break;
    case 4:
      sysMenu();
      break;
    default:
      //code placed here will run each time the loop function is called (every cpu cycle)
      break;
  }

  //reset command and parameters for next cycle
  cmdS = "-1";
  valS = "-1";
  val2S = "-1";
}
void mainMenu() {
  char cmd = cmdS.at(0); //convert command to an integer
  int val = valS.toInt(); //convert first command parameter to an integer
  int val2 = val2S.toInt(); //convert second command parameter to an integer

  //call function that matches the command
  switch (cmd) {
    case '0':
      //call function that corosponds to command #0
      showMenu(0); //display help menu over the serial communications
      break;
    case '1':
      //call function that corosponds to command #1
      currentMenu = 1;
      MotorMenu();
      break;
    case '2':
      //call function that corosponds to command #2
      currentMenu = 2;
      LEDMenu();
      break;
    case '3':
      //call function that corosponds to command #2
      currentMenu = 3;
      PIRMenu();
      break;
    default:
      //code placed here will run each time the loop function is called (every cpu cycle)
      break;
  }
}
//Process commands for Motor menu
void MotorMenu() {
  char cmd = cmdS.at(0); //convert command to a char
  int val = valS.toInt(); //convert first command parameter to an integer
  int val2 = val2S.toInt(); //convert second command parameter to an integer

  //call function that matches the command
  switch (cmd) {
    case refreshMenuChar:
      //call function that corosponds to command #0
      showMenu(1); //display help menu over the serial communications
      break;
    case returnToLastMenuChar:
      //call function that corosponds to command #1
      currentMenu = 0;
      break;
    case '1':
      //call function that corosponds to command #2
      //Set a Motor pin (MotorNumber, pinNumber)
      configureMotor(val,val2);
      break;
    case '2':
      //call function that corosponds to command #3
      //Toggle a Motor state (MotorNumber)
      toggleMotorState(!!val);
      break;
    case '3':
      //call function that corosponds to command #4
      //Set speed of a Motor (MotorNumber,Speed)
      setMotorSpeed(val,val2);
      break;
    case '4':
      //call function that corosponds to command #5
      //Gradually change a Motors speed (MotorNumber,Speed)
      rampMotorSpeed(val,val2);
      break;
    case '5':
      //call function that corosponds to command #5
      //Set default Motor speed ramping increment (RPMincrement)
      defaultRampIncrement=val;
      break;
    case '6':
      //call function that corosponds to command #5
      //Set default Motor speed ramping delay (milliseconds)
      defaultRampDelay=val;
      break;
    default:
      //code placed here will run each time the loop function is called (every cpu cycle)
      break;
  }
}
//Process commands for LED menu
void LEDMenu() {
  char cmd = cmdS.at(0); //convert command to an integer
  int val = valS.toInt(); //convert first command parameter to an integer
  int val2 = val2S.toInt(); //convert second command parameter to an integer

  //call function that matches the command
  switch (cmd) {
    case refreshMenuChar:
      //call function that corosponds to command #0
      showMenu(2); //display help menu over the serial communications
      break;
    case returnToLastMenuChar:
      //call function that corosponds to command #1
      currentMenu = 0;
      break;
    case '1':
      //call function that corosponds to command #2
      //Toggle an LED state (LEDnumber)
      toggleLEDState(val);
      break;
    case '2':
      //call function that corosponds to command #2
      //Set state of an LED (LEDnumber,0:1)
      setLEDState(val, !!val2);
      break;
    case '3':
      //call function that corosponds to command #2
      //Set an LED pin (LEDnumber, pinNumber)
      configureLED(val, val2);
      break;
    default:
      //code placed here will run each time the loop function is called (every cpu cycle)
      break;
  }
}
//Process commands for PIR menu
void PIRMenu() {
  char cmd = cmdS.at(0); //convert command to an integer
  int val = valS.toInt(); //convert first command parameter to an integer
  int val2 = val2S.toInt(); //convert second command parameter to an integer

  //call function that matches the command
  switch (cmd) {
    case refreshMenuChar:
      //call function that corosponds to command #0
      showMenu(3); //display help menu over the serial communications
      break;
    case returnToLastMenuChar:
      //call function that corosponds to command #1
      currentMenu = 0;
      break;
    case '1':
      //call function that corosponds to command #2
      //Print stored PIR output (PIRnumber)
      if((val>=0)&&(val<numberOfPIRs)){
        Serial.println("PIR #"+val+" = "+pirOutput[val]);
      }
      break;
    case '2':
      //call function that corosponds to command #2
      //Update PIR state (PIRnumber)
      if((val>=0)&&(val<numberOfPIRs)){
        getPIRState(val);
        Serial.println("New value of PIR #"+val+" = "+pirOutput[val]);
      }
      break;
    default:
      //code placed here will run each time the loop function is called (every cpu cycle)
      break;
  }
}
//Process commands for System menu
void sysMenu() {
  char cmd = cmdS.at(0); //convert command to an integer
  int val = valS.toInt(); //convert first command parameter to an integer
  int val2 = val2S.toInt(); //convert second command parameter to an integer

  Serial.println("1 = Set run mode (modeID)"); //print third command
  Serial.println("2 = Print available run modes"); //print fourth command
  Serial.println("3 = Print current pin setting"); //print fifth command

  //call function that matches the command
  switch (cmd) {
    case refreshMenuChar:
      //call function that corosponds to command #0
      showMenu(4); //display help menu over the serial communications
      break;
    case returnToLastMenuChar:
      //call function that corosponds to command #1
      currentMenu = 0;
      break;
    case '1':
      //call function that corosponds to command #2
      //Set run mode (modeID)
      runmode=val;
      break;
    case '2':
      //call function that corosponds to command #2
      //Print available run modes
      table t=new table(2);
      t.titles.addData("ID #");
      t.titles.addData("Name");
      t.addRow("0","Serial Commands only");
      t.addRow("1","Min/Max motor speed based on PIR state");
      t.addRow("2","Ramp motor speed up or down based on PIR state");
      break;
    case '3':
      //call function that corosponds to command #2
      //Print current pin setting
      table t=new table(2);
      t.titles.addData("Name");
      t.titles.addData("Pin #");
      for(int i=0;i<numberOfMotors;i++){
        t.addRow("Motor #"+String(i),String(motorPins[i]));
      }
      for(int i=0;i<numberOfLEDs;i++){
        t.addRow("LED #"+String(i),String(LEDPins[i]));
      }
      for(int i=0;i<numberOfPIRs;i++){
        t.addRow("PIR #"+String(i),String(PIRPins[i]));
      }
      break;
    default:
      //code placed here will run each time the loop function is called (every cpu cycle)
      break;
  }
}

//
//User Interface Control Functions
//

//Print text to the serial console (used for debugging)
void debug(String output){
  //verify that serial communications are configured
  if(Serial){
    //Write text to serial console if debuging is enabled and serial communications are configured
    if((debugEnabled)&&(Serial)){
      Serial.println(output);
    }
  }
}
//print a line of characters to denote seperate sections of a texted based menu over serial communications
void printLineBreak(char character, int width) {
  //verify that serial communications are configured
  if(Serial){
    //repeat the following code the number of times as sepcified in the "width" parameter
    for (int i = 0; i < width; i++) {
      Serial.print(character); //print the specified character over serial communications without starting a newline after printing
    }
    Serial.print('\n'); //end the line by sending a return carriage character
  }
}
//print a line of characters to denote seperate sections of a texted based menu over serial communications
//using the default console width and default linebreak character
void printLineBreak() {
  printLineBreak(lineBreakCharacter, consoleWidth); //pass lineBreakCharacter and default consoleWidth to printLineBreak function
}
//print a bunch of blank lines over serial communications to clear the console screen of old output
void clearConsole() {
  //verify that serial communications are configured
  if(Serial){
    //print a blank line "consoleHeight" number of times
    for (int i = 0; i < consoleHeight; i++) {
      Serial.println(); //print a blank line over serial communications
    }
  }
}
//print a text based title header for a menu over serial communications
void printHeader(String title) {
  clearConsole(); //clear old output from the serial console
  printLineBreak(); //print a line of characters to denote top of the title header
  Serial.println(title); //print the title text over serial communications
  printLineBreak(); //print a line of characters to denote bottom of the title header
}
//print a text based table row of data over serial communications
void printTableHeader(String columnTitles){

  printHeader(row);
}
//print a text based menu over serial communications
void showMenu(int menuID) {
  //figure out which menu to print
  switch (menuID) {
    case 0:
      //print the menu that corosponds with menu #0 (Help Menu)
      //print a title header
      printHeader("Main Menu");
      //print commands and descriptions
      Serial.println(String(refreshMenuChar)+" = Show current menu"); //print first command
      Serial.println(); //print a blank line

      Serial.println("1 = Motor controls"); //print second command
      Serial.println("2 = LED Controls"); //print third command
      Serial.println("3 = PIR Controls"); //print fourth command
      Serial.println("4 = System Controls"); //print fifth command
      //print a line at bottom of the menu to show denote the end of the menu
      printLineBreak();
      break;
    case 1:
      //print the menu that corosponds with menu #1 (Motor Control Menu)
      //print a title header
      printHeader("Motor Control Menu");
      //print commands and descriptions
      Serial.println(String(refreshMenuChar)+" = Show current menu"); //print first command
      Serial.println(String(returnToLastMenuChar)+" = Return to Main Menu"); //print second command
      Serial.println(); //print a blank line

      //check if there are any Motors's in the system
      if (numberOfMotors > 0) {
        Serial.println("1 = Set a Motor pin (MotorNumber, pinNumber)"); //print third command
        Serial.println("2 = Toggle a Motor state (MotorNumber)"); //print fourth command
        Serial.println("3 = Set speed of a Motor (MotorNumber,Speed)"); //print fifth command
        Serial.println("4 = Gradually change a Motors speed (MotorNumber,Speed)"); //print sixth command
        Serial.println("5 = Set default Motor speed ramping increment (RPMincrement)"); //print seventh command
        Serial.println("6 = Set default Motor speed ramping delay (milliseconds)"); //print eigth command
      } else {
        //print a blank line
        Serial.println();

        //display message warning that there are no Motors in the system
        Serial.println("There are no Motors in the current system");
      }
      //print a line at bottom of the menu to show denote the end of the menu
      printLineBreak();
      break;
    case 2:
      //print the menu that corosponds with menu #2 (LED Control Menu)
      //print a title header
      printHeader("LED Control Menu");
      //print commands and descriptions
      Serial.println(String(refreshMenuChar)+" = Show current menu"); //print first command
      Serial.println(String(returnToLastMenuChar)+" = Return to Main Menu"); //print second command
      Serial.println(); //print a blank line

      //check if there are any LED's in the system
      if (numberOfLEDs > 0) {
        Serial.println("1 = Toggle an LED state (LEDnumber)"); //print third command
        Serial.println("2 = Set state of an LED (LEDnumber,0:1)"); //print fourth command
        Serial.println("3 = Set an LED pin (LEDnumber, pinNumber)"); //print fifth command
      } else {
        //print a blank line
        Serial.println();

        //display message warning that there are no LED's in the system
        Serial.println("There are no LED's in the current system");
      }
      //print a line at bottom of the menu to show denote the end of the menu
      printLineBreak();
      break;
    case 3:
      //print the menu that corosponds with menu #2 (LED Control Menu)
      //print a title header
      printHeader("PIR Control Menu");
      //print commands and descriptions
      Serial.println(String(refreshMenuChar)+" = Show current menu"); //print first command
      Serial.println(String(returnToLastMenuChar)+" = Return to Main Menu"); //print second command
      Serial.println(); //print a blank line

      //check if there are any LED's in the system
      if (numberOfPIRs > 0) {
        Serial.println("1 = Print stored PIR output (PIRnumber)"); //print third command
        Serial.println("2 = Update PIR state (PIRnumber)"); //print fourth command
        //Serial.println("4 = Set an LED pin (LEDnumber, pinNumber)"); //print fifth command
      } else {
        //print a blank line
        Serial.println();

        //display message warning that there are no LED's in the system
        Serial.println("There are no PIR's in the current system");
      }
      //print a line at bottom of the menu to show denote the end of the menu
      printLineBreak();
      break;
    case 4:
      //print the menu that corosponds with menu #2 (LED Control Menu)
      //print a title header
      printHeader("System Control Menu");
      //print commands and descriptions
      Serial.println(String(refreshMenuChar)+" = Show current menu"); //print first command
      Serial.println(String(returnToLastMenuChar)+" = Return to Main Menu"); //print second command
      Serial.println(); //print a blank line

      Serial.println("1 = Set run mode (modeID)"); //print third command
      Serial.println("2 = Print available run modes"); //print fourth command
      Serial.println("3 = Print current pin setting"); //print fifth command

      //print a line at bottom of the menu to show denote the end of the menu
      printLineBreak();
      break;
    default:
      break;
  }
}
