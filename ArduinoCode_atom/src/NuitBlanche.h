/*
<<<File Info>>>
Description: Open Source code suite for Nuit Blanche style projects

Developer: Jonathan Brunath

Date Created: 4/8/10/2015
Updated: 7/11/12/2016
Updated: 1/12/12/2016
Updated: 2/13/12/2016
*/

#include "Arduino.h"
//#include "tables.h"

//
//Variable Definitions
//

bool debugEnabled = true; //toggles debugging output over serial communications

//constants (values that can not be changed while the system is running)
const int numberOfMotors = 1; //defines the number of motors in the system
const int numberOfLEDs = 1; //defines the number of LED's in the system
const int numberOfPIRs = 1; //defines the number of PIR's in the system

//limit variables
int maxDuration = 1000; //defines the maximum duration in milliseconds motor speed ramping function

//pinout
int LEDPins[numberOfLEDs]; //array to store led I/O pin numbers
int PIRPins[numberOfLEDs]; //array to store PIR I/O pin numbers

//defaults
int defaultMotorMaxRPM = 150; //defines the default maximum RPM of the motors
int defaultLEDPins[] = {2}; //defines the default LED pin numbers (number of pins can differ from value of "numberOfLEDs", extras will be ignored during configuration)
int defaultMotorForwardPins[] = {3,6,10}; //defines the default Motor Forward pin numbers (number of pins can differ from value of "numberOfMotors", extras will be ignored during configuration)
int defaultMotorReversePins[] = {5,9,11}; //defines the default Motor Reverse pin numbers (number of pins can differ from value of "numberOfMotors", extras will be ignored during configuration)
int defaultPIRPins[] = {4}; //defines the default PIR pin numbers (number of pins can differ from value of "numberOfPIRs", extras will be ignored during configuration)
int defaultUpperTargetMotorSpeed = defaultMotorMaxRPM; //defines the default upper target speed of the motors
int defaultLowerTargetMotorSpeed = 0; //defines the default lower target speed of the motors
int defaultRampDuration = 500; //defines the default ramping duration in milliseconds for ramping the speed of a motor
bool defaultLEDState = true; //defines the default state for the LED's
bool defaultMotorState = false; //defines the default state for the motors
int defaultMotorPolarity = 1; //defines the default polarity for the motors

//state variables
bool LEDState[numberOfLEDs]; //array to store current state of the LED's
bool pirOutput[numberOfPIRs]; //array to store the output of each PIR sensor

//motor object
class motorObj{
  private:
    bool motorActive;
    bool motorPolarity[2]; //boolean to store polarity states of the motor (direction of spin)
  public:
    int motorPolarityPins[2]; //integer to store motor polarity I/O pin numbers
    int motorSpeed; //integer to store the speed of the motor
    int rampDuration; //stores the ramping increment value for the motor;
    int maxRPM; //stores the maximum safe RPM of the motor

    motorObj(){
      motorPolarityPins[0]=3;
      motorPolarityPins[1]=5;
      motorActive=false;
      motorPolarity[0]=false;
      motorPolarity[1]=false;
      motorSpeed=0;
      rampDuration=1;
      maxRPM=150;
    }

    //boolean to store current state of the motor
    bool *getMotorPolarity(int polarity){
      if((polarity==0)||(polarity==1)){
        return &motorPolarity[polarity];
      } else {
        return &motorActive;
      }
    }
    int getMotorPolarity(){
      if(motorPolarity[0]){
        return 1;
      } else if(motorPolarity[1]){
        return -1;
      } else {
        return 0;
      }
    }
    //boolean to store current state of the motor
    bool *motorState(){
      motorActive=((motorPolarity[0])^(motorPolarity[1]));
      return &motorActive;
    }
    //return the pin number of the currently selected direction of spin
    int *motorSpeedPin(){
      if(motorPolarity[0]){
        return &motorPolarityPins[0];
      }else if (motorPolarity[1]){
        return &motorPolarityPins[1];
      } else {
        return &motorPolarityPins[0];
      }
    }
    void setMotorPolarity(int polarity){
      if(polarity>0){
        motorPolarity[0]=true;
        motorPolarity[1]=false;
      } else if(polarity<0){
        motorPolarity[0]=false;
        motorPolarity[1]=true;
      } else {
        motorPolarity[0]=false;
        motorPolarity[1]=false;
      }
    }
};
motorObj motors[numberOfMotors];

//command processing variables
String cmdS; //String to store incoming command
String valS; //String to store first command parameter
String val2S; //String to store first command parameter
String val3S; //String to store first command parameter
char seperator = ','; //character to use to differentiate between the command and parameters in an incoming serial String (command)
char delimiter = '\n'; //character to use to signify the end of an incoming serial String (command)
bool cmdsEnabled = true; //boolean to store weather commands are enabled and will be processed or not
int baudRate = 9600; //integer to store the baud rate to use for serial communications (similar to transfer speed)
int runMode = 0; //defines which mode to run the system in

//menu variables
int consoleWidth = 40; //defines the default number of characters in the width of serial communications output console
int consoleHeight = 80; //defines the default number of characters in the height of serial communications output console
char lineBreakCharacter = '-'; //defines the default character to print when printing a line break to denote different sections of a texted based menu over serial communications
int currentMenu = 0; //denotes which menu is currently selected
char refreshMenuChar = 'r';
char returnToLastMenuChar = 'b';

//Print text to the serial console (used for debugging)
void debug(String output) {
  //verify that serial communications are configured
  if (Serial) {
    //Write text to serial console if debuging is enabled and serial communications are configured
    if ((debugEnabled) && (Serial)) {
      Serial.println(output);
    }
  }
}

//
//I/O Control Functions
//

//set digital pin on or off
void setDigitalOutput(int *pinNumber, bool state, bool *storedState) {
  //determine weather to turn pin "pinNumber" on or off
  if (state) {
    digitalWrite(*pinNumber, HIGH); //set pin "pinNumber" to ON
  } else {
    digitalWrite(*pinNumber, LOW); //set pin "pinNumber" to OFF
  }
  //set stored state variable
  *storedState = state;
}
//toggle digital pin on or off
void toggleDigialOutput(int *pinNumber, bool *storedState) {
  //set pin "pinNumber" state to the inverse of the on current pin state as specified by the stored state value "storedState"
  setDigitalOutput(pinNumber, !*storedState, storedState);
}
//set analog pin on or off
void setAnalogOutput(int *pinNumber, int output, int *storedOutput) {
  //set the analog output to the specified value
  analogWrite(*pinNumber, output);

  //set stored state variable
  *storedOutput = output;
}
//configure new digital output pin
void configureDigitalOutputPin(int *pinNumber, bool *storedState, bool *defaultState) {
  //set digital pin "pinNumber" as an output
  pinMode(*pinNumber, OUTPUT);

  //set new pin number to OFF state and update the pins corosponding stored state variable
  setDigitalOutput(pinNumber, defaultState, storedState);
}
//configure new digital input pin
void configureDigitalInputPin(int *pinNumber, bool *storedState) {
  //set digital pin "pinNumber" as an input
  pinMode(*pinNumber, INPUT);

  //read the initial input value and store it in the storedState variable
  *storedState = digitalRead(*pinNumber);
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
    configureDigitalInputPin(&PIRPins[pirNumber], &pirOutput[pirNumber]);
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
void configureMotor(int motorNumber, int forwardPinNumber, int reversePinNumber, int polarity = defaultMotorPolarity) {
  //check if specified pirNumber is valid
  if ((motorNumber >= 0) && (motorNumber < numberOfMotors)) {
    //set "motorPins" array index "motorNumber" to equal "pinNumber"
    motors[motorNumber].motorPolarityPins[0] = forwardPinNumber;
    motors[motorNumber].motorPolarityPins[1] = reversePinNumber;

    //pass motor pin number, stored state variable, and inital state to "configureDigitalOutputPin"
    configureDigitalOutputPin(&motors[motorNumber].motorPolarityPins[0], motors[motorNumber].getMotorPolarity(0), &defaultMotorState);
    configureDigitalOutputPin(&motors[motorNumber].motorPolarityPins[1], motors[motorNumber].getMotorPolarity(1), &defaultMotorState);
    //Set default Motor initial motor speed to zero
    setAnalogOutput(motors[motorNumber].motorSpeedPin(), 0, &motors[motorNumber].motorSpeed);
  }
}
//Set default Motor on or off (speed=0 or speed=max)
void setMotorState(int motorNumber, bool state) {
  //check if specified motorNumber is valid
  if ((motorNumber >= 0) && (motorNumber < numberOfMotors)) {
    //pass specified motor pin number and stored motor state along with the new state to "setOuput" function
    setDigitalOutput(motors[motorNumber].motorSpeedPin(), state, motors[motorNumber].motorState());
  }
}
//convert an RPM value to analog output range
int rpmToAnalog(int motorNumber, int rpm) {
  //check if specified motorNumber is valid
  if ((motorNumber >= 0) && (motorNumber < numberOfMotors)) {
    //map the RPM range of the specified motor to the range of the analog output
    return map(rpm, 0, motors[motorNumber].maxRPM, 0, 255);
  }

  //return a value of -1 to signify that the specified motor number was invalid
  return -1;
}
//set the speed of a specific motor
void setMotorSpeed(int motorNumber, int speed) {
  //check if specified motorNumber is valid
  if ((motorNumber >= 0) && (motorNumber < numberOfMotors)) {
    //check if specified speed is valid
    if ((speed >= 0) && (speed <= motors[motorNumber].maxRPM)) {
      //pass specified motor pin number and stored motor speed along with the new state to "setOuput" function
      setAnalogOutput(motors[motorNumber].motorSpeedPin(), rpmToAnalog(motorNumber, speed), &motors[motorNumber].motorSpeed);
    }
  }
}
//toggle motor state
void toggleMotorState(int motorNumber) {
  //check if specified motorNumber is valid
  if ((motorNumber >= 0) && (motorNumber < numberOfMotors)) {
    //pass specified motor pin number and stored motor state to "toggleOuput" function
    toggleDigialOutput(motors[motorNumber].motorSpeedPin(), motors[motorNumber].motorState());
  }
}
//set a specific motor's maximum RPM
void setMaxRPM(int motorNumber, int max) {
  //check if specified motorNumber is valid
  if ((motorNumber >= 0) && (motorNumber < numberOfMotors)) {
    motors[motorNumber].maxRPM = max;
  }
}
//change a motors speed gradually over time
void rampMotorSpeed(int motorNumber, int targetSpeed, int duration) {
  //check if specified motorNumber is valid
  if ((motorNumber >= 0) && (motorNumber < numberOfMotors)) {
    //calculate increment value from duration and max RPM of specified motor
    int increment = motors[motorNumber].maxRPM/duration;

    debug("");
    debug("Current Speed: " + String(motors[motorNumber].motorSpeed));
    debug("Target Speed: " + String(targetSpeed));
    debug("Duration: " + String(duration));
    debug("Increment: " + String(increment));

    //determine weather the speed needs to ramp up or down
    if (motors[motorNumber].motorSpeed < targetSpeed) {
      //debug("");
      //debug("ramp up");
      //debug("");
      //delay(1000);

      //start at initial motor speed, incrementing up by the specified value until the targetSpeed is reached
      for (int i = motors[motorNumber].motorSpeed; i <= targetSpeed; i += increment) {
        debug("RPM: " + String(i));
        debug("Analog Output: " + String(rpmToAnalog(motorNumber, i)));
        debug("");
        setAnalogOutput(motors[motorNumber].motorSpeedPin(), rpmToAnalog(motorNumber, i), &motors[motorNumber].motorSpeed);
      }
    } else if (motors[motorNumber].motorSpeed > targetSpeed) {
      //debug("");
      //debug("ramp down");
      //debug("");
      //delay(1000);
      //start at initial motor speed, incrementing down by the specified value until the targetSpeed is reached
      for (int i = motors[motorNumber].motorSpeed; i >= targetSpeed; i -= increment) {
        debug("RPM: " + String(i));
        debug("Analog Output: " + String(rpmToAnalog(motorNumber, i)));
        debug("");
        setAnalogOutput(motors[motorNumber].motorSpeedPin(), rpmToAnalog(motorNumber, i), &motors[motorNumber].motorSpeed);

        /*
        if ((delayTime > 1) && (delayTime < maxDelay)) {
          delay(delayTime);
        }
        */
      }
    }
  }
  motors[motorNumber].motorSpeed = targetSpeed;
}

//call rampMotorSpeed function using the default increment and delay values
void rampMotorSpeed(int motorNumber, int targetSpeed) {
  rampMotorSpeed(motorNumber, targetSpeed, defaultRampDuration);
}
//call rampMotorSpeed function using the default upper target motor speed and ramp duration
void rampMotorSpeedUp(int motorNumber) {
  rampMotorSpeed(motorNumber, defaultUpperTargetMotorSpeed, defaultRampDuration);
}
//call rampMotorSpeed function using the default lower target motor speed and ramp duration
void rampMotorSpeedDown(int motorNumber) {
  rampMotorSpeed(motorNumber, defaultLowerTargetMotorSpeed, defaultRampDuration);
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
    configureDigitalOutputPin(&LEDPins[LEDNumber], &LEDState[LEDNumber], &defaultLEDState);
  }
}
//set LED on or off
void setLEDState(int LEDNumber, bool state) {
  //pass specified LED pin number and stored LED state along with the new state to "setOuput" function
  setDigitalOutput(&LEDPins[LEDNumber], state, &LEDState[LEDNumber]);
}
//toggle LED state
void toggleLEDState(int LEDNumber) {
  //pass specified LED pin number and stored LED state to "toggleOuput" function
  toggleDigialOutput(&LEDPins[LEDNumber], &LEDState[LEDNumber]);
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
//User Interface Control Functions
//

//print a line of characters to denote seperate sections of a texted based menu over serial communications
void printLineBreak(char character, int width) {
  //verify that serial communications are configured
  if (Serial) {
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
  if (Serial) {
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
//print a text based menu over serial communications
void showMenu(int menuID) {
  //figure out which menu to print
  switch (menuID) {
    case 0:
      //print the menu that corosponds with menu #0 (Help Menu)
      //print a title header
      printHeader("Main Menu");
      //print commands and descriptions
      Serial.println(String(refreshMenuChar) + " = Show current menu"); //print first command
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
      Serial.println(String(refreshMenuChar) + " = Show current menu"); //print first command
      Serial.println(String(returnToLastMenuChar) + " = Return to Main Menu"); //print second command
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
      Serial.println(String(refreshMenuChar) + " = Show current menu"); //print first command
      Serial.println(String(returnToLastMenuChar) + " = Return to Main Menu"); //print second command
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
      Serial.println(String(refreshMenuChar) + " = Show current menu"); //print first command
      Serial.println(String(returnToLastMenuChar) + " = Return to Main Menu"); //print second command
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
      Serial.println(String(refreshMenuChar) + " = Show current menu"); //print first command
      Serial.println(String(returnToLastMenuChar) + " = Return to Main Menu"); //print second command
      Serial.println(); //print a blank line

      Serial.println("1 = Set run mode (modeID)"); //print third command
      Serial.println("2 = Print available run modes"); //print fourth command
      Serial.println("3 = Print current pin setting"); //print fifth command
      Serial.println("4 = Set console width (width)"); //print sixth command
      Serial.println("5 = Set console height (height)"); //print sixth command
      Serial.println("6 = toggle debug mode"); //print sixth command

      //print a line at bottom of the menu to show denote the end of the menu
      printLineBreak();
      break;
    default:
      break;
  }
}
//Switch between menus
void switchMenus(int menuID) {
  currentMenu = menuID;
  showMenu(currentMenu);
}

//
//Command Processing
//

void mainMenu() {
  char cmd = cmdS.charAt(0); //convert command to an integer
  //int val = valS.toInt(); //convert first command parameter to an integer
  //int val2 = val2S.toInt(); //convert second command parameter to an integer
  if (cmdS != "-1") {
    //debug("Command Char: " + String(cmd));
    //debug("Val int: " + String(val));
    //debug("Val2 int: " + String(val2));
  }
  //call function that matches the command
  if (cmd == refreshMenuChar) {
    //call function that corosponds to command #0
    showMenu(currentMenu); //display help menu over the serial communications
  } else if (cmd == '1') {
    //call function that corosponds to command #1
    switchMenus(1);
  } else if (cmd == '2') {
    //call function that corosponds to command #2
    switchMenus(2);
  } else if (cmd == '3') {
    //call function that corosponds to command #3
    switchMenus(3);
  } else if (cmd == '4') {
    //call function that corosponds to command #4
    switchMenus(4);
  } else {
    if (cmdS != "-1") {
      //debug("Invalid Command");
    }
  }
}
//Process commands for Motor menu
void MotorMenu() {
  char cmd = cmdS.charAt(0); //convert command to a char
  int val = valS.toInt(); //convert first command parameter to an integer
  int val2 = val2S.toInt(); //convert second command parameter to an integer
  int val3 = val3S.toInt(); //convert third command parameter to an integer

  //call function that matches the command
  if (cmd == refreshMenuChar) {
    //call function that corosponds to command #0
    showMenu(1); //display help menu over the serial communications

  } else if (cmd == returnToLastMenuChar) {
    //call function that corosponds to command #1
    switchMenus(0);

  } else if (cmd == '1') {
    //call function that corosponds to command #2
    //Set a Motor pin (MotorNumber, pinNumber)
    configureMotor(val, val2, val3);
    Serial.println("Motor #" + String(val) + " pin = " + String(val2));
  } else if (cmd == '2') {
    //call function that corosponds to command #3
    //Toggle a Motor state (MotorNumber)
    toggleMotorState(!!val);
    Serial.println("Motor #" + String(val) + " = " + String(!!val2));
  } else if (cmd == '3') {
    //call function that corosponds to command #4
    //Set speed of a Motor (MotorNumber,Speed)
    setMotorSpeed(val, val2);
    Serial.println("Motor #" + String(val) + "Direction = " + String(motors[val].getMotorPolarity()));
    Serial.println("Motor #" + String(val) + "Speed = " + String(val2));
  } else if (cmd == '4') {
    //call function that corosponds to command #5
    //Gradually change a Motors speed (MotorNumber,Speed)
    rampMotorSpeed(val, val2);
    Serial.println("Motor #" + String(val) + " = " + String(val2));
  } else if (cmd == '5') {
    //call function that corosponds to command #5
    //Set default Motor speed ramping increment (RPMincrement)
    defaultRampDuration = val;
    Serial.println("Default Motor Ramp Duration = " + String(defaultRampDuration));
  } else if (cmd == '6') {
    //call function that corosponds to command #5
    //Set Motor direction ramping delay (milliseconds)
    if((val>=0)&&(val<numberOfMotors)){
      motors[val].setMotorPolarity(val2);

    }
  } else {
    if (cmdS != "-1") {
      //debug("Invalid Command");
    }
  }
}

//Process commands for LED menu
void LEDMenu() {
  char cmd = cmdS.charAt(0); //convert command to an integer
  int val = valS.toInt(); //convert first command parameter to an integer
  int val2 = val2S.toInt(); //convert second command parameter to an integer

  //call function that matches the command
  if (cmd == refreshMenuChar) {
    //call function that corosponds to command #0
    showMenu(2); //display help menu over the serial communications

  } else if (cmd == returnToLastMenuChar) {
    //call function that corosponds to command #1
    switchMenus(0);

  } else if (cmd == '1') {
    //call function that corosponds to command #2
    //Toggle an LED state (LEDnumber)
    toggleLEDState(val);
    Serial.println("LED #" + String(val) + " state = " + String(LEDState[val]));
  } else if (cmd == '2') {
    //call function that corosponds to command #2
    //Set state of an LED (LEDnumber,0:1)
    setLEDState(val, !!val2);
    Serial.println("LED #" + String(val) + " state = " + String(val2));

  } else if (cmd == '3') {
    //call function that corosponds to command #2
    //Set an LED pin (LEDnumber, pinNumber)
    configureLED(val, val2);
    Serial.println("LED #" + String(val) + " pin = " + String(val2));

  } else {
    if (cmdS != "-1") {
      //debug("Invalid Command");
    }
  }
}

//Process commands for PIR menu
void PIRMenu() {
  char cmd = cmdS.charAt(0); //convert command to an integer
  int val = valS.toInt(); //convert first command parameter to an integer
  //int val2 = val2S.toInt(); //convert second command parameter to an integer
  //int val3 = val3S.toInt(); //convert second command parameter to an integer

  //call function that matches the command

  if (cmd == refreshMenuChar) {
    //call function that corosponds to command #0
    showMenu(3); //display help menu over the serial communications
  } else if (cmd == returnToLastMenuChar) {
    //call function that corosponds to command #1
    switchMenus(0);
  } else if (cmd == '1') {
    //call function that corosponds to command #2
    //Print stored PIR output (PIRnumber)
    if ((val >= 0) && (val < numberOfPIRs)) {
      Serial.println("PIR #" + String(val) + " = " + String(pirOutput[val]));
    }
  } else if (cmd == '2') {
    //call function that corosponds to command #2
    //Update PIR state (PIRnumber)
    if ((val >= 0) && (val < numberOfPIRs)) {
      getPIRState(val);
      Serial.println("New value of PIR #" + String(val) + " = " + String(pirOutput[val]));
    }
  } else {
    if (cmdS != "-1") {
      //debug("Invalid Command");
    }
  }
}
//Process commands for System menu
void sysMenu() {
  char cmd = cmdS.charAt(0); //convert command to an integer
  int val = valS.toInt(); //convert first command parameter to an integer
  //int val2 = val2S.toInt(); //convert second command parameter to an integer

  //call function that matches the command
  if (cmd == refreshMenuChar) {
    //call function that corosponds to command #0
    showMenu(4); //display help menu over the serial communications

  } else if (cmd == returnToLastMenuChar) {
    //call function that corosponds to command #1
    switchMenus(0);

  } else if (cmd == '1') {
    //call function that corosponds to command #2
    //Set run mode (modeID)
    runMode = val;
    Serial.println("Run Mode = " + String(runMode));

  } else if (cmd == '2') {
    //call function that corosponds to command #3
    //Print available run modes

    /*
    tableObj t(2);
    t.titles->addData("ID #");
    t.titles->addData("Name");
    t.addRow("0", "Serial Commands only");
    t.addRow("1", "Min/Max motor speed based on PIR state");
    t.addRow("2", "Ramp motor speed up or down based on PIR state");
    delete &t;
    */
  } else if (cmd == '3') {
    //call function that corosponds to command #4
    //Print current pin setting
    /*
    tableObj t(2);
    t.titles->addData("Name");
    t.titles->addData("Pin #");
    t.titles->print();
    for (int i = 0; i < numberOfMotors; i++) {
      t.addRow("Motor #" + String(i), String(motorPins[i]));
    }
    for (int i = 0; i < numberOfLEDs; i++) {
      t.addRow("LED #" + String(i), String(LEDPins[i]));
    }
    for (int i = 0; i < numberOfPIRs; i++) {
      t.addRow("PIR #" + String(i), String(PIRPins[i]));
    }
    delete &t;
    */
  } else if (cmd == '4') {
    //call function that corosponds to command #5
    //Set console width (width)
    if (val > 0) {
      consoleWidth = val;
      Serial.println("Console Width = " + String(consoleWidth));
    }
  } else if (cmd == '5') {
    //call function that corosponds to command #6
    //Set console height (height)
    if (val > 0) {
      consoleHeight = val;
      Serial.println("Console Height = " + String(consoleHeight));
    }
  } else if (cmd == '6') {
    //call function that corosponds to command #7
    //toggle debug mode
    debugEnabled = !debugEnabled;
    if (debugEnabled) {
      Serial.println("Debug enabled");
    } else {
      Serial.println("Debug disabled");
    }
  }
}
//process commands from the serial port
void processCmds() {
  /*
  NOTICE: the way commands are current parsed can be made much more efficent. The
  code will be updated if the need arises and time allows for it.
  */

  //read incoming command with parameters if serial port is available(active)
  while (Serial.available()) {
    //debug("Reading command...");

    cmdS = Serial.readStringUntil(seperator); //read command
    valS = Serial.readStringUntil(seperator); //read first command parameter
    val2S = Serial.readStringUntil(seperator); //read second command parameter
    val3S = Serial.readStringUntil(delimiter); //read third command parameter

    debug("Command: " + cmdS);
    debug("Val: " + valS);
    debug("Val2: " + val2S);
    debug("Val3: " + val3S);
    debug("Current Menu: " + String(currentMenu));
    debug("");

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
  val3S = "-1";
}

//
//Core System Functions
//

void initializeMotors(){
  //configure motors
  //update motorObj array with default pin values
  for (int i = 0; i < numberOfMotors; i++) {
    configureMotor(i, defaultMotorForwardPins[i], defaultMotorReversePins[i],0);
  }
}
void initializeLEDs(){
  //configure LED's
  //update LEDPins array with default pin values and set new pins numbers as outputs
  for (int i = 0; i < numberOfLEDs; i++) {
    configureLED(i, defaultLEDPins[i]);
  }
}
void initializePIRs(){
  //configure PIR's
  //update PIRPins array with default pin values and set new pins numbers as inputs
  for (int i = 0; i < numberOfPIRs; i++) {

    //debug("Default PIR Pin: " + defaultPIRPins[i]);
    configurePIR(i, defaultPIRPins[i]);
  }
}
