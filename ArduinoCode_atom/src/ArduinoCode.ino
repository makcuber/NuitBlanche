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

//initialize system settings and start basic processes
void setup() {
  serialControl(); //enable serial port if commands are enabled

  initializeMotors();
  initializeLEDs();
  initializePIRs();

  //if commands are enabled show the main menu on startup
  if (cmdsEnabled) {
    showMenu(0);
  }
}

//repeat the following code block for each cycle of the cpu
void loop() {
  //process incoming commands from serial port if enabled
  if (cmdsEnabled) {
    processCmds();
  }

  if (runMode > 0) {
    //debug("PIR State: " + getPIRState(0));
    //debug("");
    //delay(100);
  }

  //determine which mode to run the system in
  if (runMode == 1) {
    //run system using basic on/off control for the motors
    for (int i = 0; i < numberOfMotors; i++) {
      setMotorState(i, pirOutput[0]); //set the motor state to the output of the PIR sensor
    }
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
