/*
<<<File Info>>>
Description: Open Source code suite for Nuit Blanche style projects

Developer: Jonathan Brunath

Date Created: 2/13/12/2016
*/

#include "Arduino.h"
#include "NuitBlanche.h"

//
//Core System Functions
//
int ledPin=3;

//initialize system settings and start basic processes
void setup() {
  //cmdsEnabled=false;
  serialControl(); //enable serial port if commands are enabled

  initializeMotors();
  initializeLEDs();
  initializePIRs();

  //if commands are enabled show the main menu on startup
  if (cmdsEnabled) {
    showMenu(0);
  }
  runMode=1;
}

//repeat the following code block for each cycle of the cpu
void loop() {
  //process incoming commands from serial port if enabled
  if (cmdsEnabled) {
    processCmds();
  }

  if (runMode > 0) {
    debug("PIR State: " + getPIRState(0));
    //debug("");
    //delay(100);
  }

  //determine which mode to run the system in
  if (runMode == 1) {
    //run system using basic on/off control for the motors
    for (int i = 0; i < numberOfMotors; i++) {
      setMotorState(i, !pirOutput[0]); //set the motor state to the output of the PIR sensor
    }
  } 
}
