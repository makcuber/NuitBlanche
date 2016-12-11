/*
<<<File Info>>>
Description: Starter code for a project

Developer: Jonathan Brunath

Date Created: 4/8/10/2015
Updated: 7/11/12/2016
*/

//
//Variable Definitions
//

//constants (values that can not be changed while the system is running)
const int numberOfMotors=1; //defines the number of motors in the system
const int numberOfLEDs=10; //defines the number of leds in the system

//pinout
int motorPins[numberOfMotors]; //array to store motor I/O pin numbers
int LEDPins[numberOfLEDs]; //array to store led I/O pin numbers
int defaultLEDPins[]={3,4,5,6,7,8,9,10,11,12}; //defines the default LED pin numbers (number of pins can differ from value of "numberOfLEDs", extras will be ignored during configuration)
int defaultMotorPins[]={2,3,4,5,6,7,8,9,10,11}; //defines the default Motor pin numbers (number of pins can differ from value of "numberOfLEDs", extras will be ignored during configuration)

//general variables
bool motorState[numberOfMotors]; //array to store current state of the motors
bool LEDState[numberOfLEDs]; //array to store current state of the LED's

//command processing variables
String cmdS; //string to store incoming command
String valS; //string to store first command parameter
char seperator=','; //character to use to differentiate between the command and parameters in an incoming serial string (command)
char delimiter='\n'; //character to use to signify the end of an incoming serial string (command)
bool cmdsEnabled=true; //boolean to store weather commands are enabled and will be processed or not
int baudRate=9600; //integer to store the baud rate to use for serial communications (similar to transfer speed)

//menu variables
int consoleWidth=25; //defines the default number of characters in the width of serial communications output console
char lineBreakCharacter='-'; //defines the default character to print when printing a line break to denote different sections of a texted based menu over serial communications

//
//Core System Functions
//

//initialize system settings and start basic processes
void setup() {
  serialControl(); //enable serial port if commands are enabled

  //configure motors
  //update motorPins array with default pin values and set new pins numbers as outputs
  for(int i=0;i<numberOfMotors;i++){
    configureMotor(i,defaultMotorPins[i]);
  }

  //configure LED's
  //update LEDPins array with default pin values and set new pins numbers as outputs
  for(int i=0;i<numberOfLEDs;i++){
    configureMotor(i,defaultLEDPins[i]);
  }
}

//repeat the following code block for each cycle of the cpu
void loop() {
  //process incoming commands from serial port if enabled
  if(cmdsEnabled){
    processCmds();
  }
  
}

//
//I/O Control Functions
//

//toggle digital pin on or off
void toggleOutput(int pinNumber, bool *storedState){
  //set pin "pinNumber" state to the inverse of the on current pin state as specified by the stored state value "storedState"
  setOutput(pinNumber, !*storedState, storedState);
}
//set digital pin on or off
void setOutput(int pinNumber, bool state, bool *storedState){
  //determine weather to turn pin "pinNumber" on or off
  if(state){
    digitalWrite(pinNumber,HIGH); //set pin "pinNumber" to ON
  } else {
    digitalWrite(pinNumber,LOW); //set pin "pinNumber" to OFF
  }
  //set stored state variable
  *storedState=state;
}
//configure new digital output pin
void configureDigitalOutputPin(int pinNumber, bool *storedState){
  pinMode(pinNumber, OUTPUT); //set digital pin "pinNumber" as an output
  setOutput(pinNumber,false,storedState); //set new pin number to OFF state and update the pins corosponding stored state variable
}

//
//Motor Control Functions
//

//Configure specific motor pin
void configureMotor(int motorNumber, int pinNumber){
  motorPins[motorNumber] = pinNumber; //set "motorPins" array index "motorNumber" to equal "pinNumber"
  configureDigitalOutputPin(motorPins[motorNumber],&motorState[motorNumber]); //pass motor pin and stored state to "configureDigitalOutputPin" function
}
//set motor on or off
void setMotorState(int motorNumber, bool state){
  //pass specified motor pin number and stored motor state along with the new state to "setOuput" function
  setOutput(motorPins[motorNumber], state, &motorState[motorNumber]);
}
//toggle motor state
void toggleMotorState(int motorNumber){
  //pass specified motor pin number and stored motor state to "toggleOuput" function
  toggleOutput(motorPins[motorNumber], &motorState[motorNumber]);
}

//
//LED Control Functions
//

//Configure specific LED pin
void configureLED(int LEDNumber, int pinNumber){
  LEDPins[LEDNumber] = pinNumber; //set "LEDPins" array index "LEDNumber" to equal "pinNumber"
  configureDigitalOutputPin(LEDPins[LEDNumber],&LEDState[LEDNumber]); //pass LED pin and stored state to "configureDigitalOutputPin" function
}
//set LED on or off
void setLEDState(int LEDNumber, bool state){
  //pass specified LED pin number and stored LED state along with the new state to "setOuput" function
  setOutput(LEDPins[LEDNumber], state, &LEDState[LEDNumber]);
}
//toggle LED state
void toggleLEDState(int LEDNumber){
  //pass specified LED pin number and stored LED state to "toggleOuput" function
  toggleOutput(LEDPins[LEDNumber], &LEDState[LEDNumber]);
}

//
//Serial Control functions
//

//Start or stop serial port
void serialControl(bool state){
  if(state){
    Serial.begin(baudRate);
  } else{
    Serial.end();
  }
}
//Start or stop serial port if commands are enabled
void serialControl(){
  serialControl(cmdsEnabled);
}

//
//Command Processing
//

//process commands from the serial port
void processCmds(){
  //read incoming command with parameters if serial port is available(active)
  while (Serial.available()) {
    cmdS = Serial.readStringUntil(seperator); //read command
    valS = Serial.readStringUntil(delimiter); //read first command parameter
  }
  int cmd = cmdS.toInt(); //convert command to an integer
  int val = valS.toInt(); //convert first command parameter to an integer

  //call function that matches the command
  switch (cmd) {
    case 0:
      //call function that corosponds to command #0
      showMenu(0); //display help menu over the serial communications
      break;
    case 1:
      //call function that corosponds to command #1
      break;
    case 2:
      //call function that corosponds to command #2
      break;
    default:
      //code placed here will run each time the loop function is called (every cpu cycle)
      break;
  }

  //reset command and parameters for next cycle
  cmdS = "-1";
  valS = "-1";
}
//print a line of characters to denote seperate sections of a texted based menu over serial communications
void printLineBreak(char character, int width){
  //repeat the following code the number of times as sepcified in the "width" parameter
  for(int i=0;i<width;i++){
    Serial.print(character); //print the specified character over serial communications without starting a newline after printing
  }
  Serial.print('\n'); //end the line by sending a return carriage character
}
//print a line of characters to denote seperate sections of a texted based menu over serial communications
//using the default console width and default linebreak character
void printLineBreak(){
  printLineBreak(lineBreakCharacter, consoleWidth);
}
//print a text based menu over serial communications
void showMenu(int menuID) {
  //figure out which menu to print
  switch (menuID) {
    case 0:
      //print the menu that corosponds with menu #0 (Help Menu)
      //print a title header
      printLineBreak();
      Serial.println("Help");
      printLineBreak();
      //print commands and descriptions
      Serial.println("0 = Show help menu"); //print first command
      Serial.println("1 = "); //print second command
      Serial.println("2 = "); //print third command
      //print a line at bottom of the menu to show denote the end of the menu
      printLineBreak();
      break;
    default:
      break;
  }
}
