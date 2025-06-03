#pragma once

#include <Arduino.h>
#include <M5Unified.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <VL53L0X.h>
#include <Unit_Sonic.h>
#include <M5_ADS1115.h>
#include <HX711.h>

#include "timeHelper.h"
#include "sensorHelper.h"
#include "BLEHelper.h"
#include "displayHelper.h"

//hold pin 4 high to keep the power on
#define POWER_HOLD_PIN 4
void powerOff(){
    digitalWrite(POWER_HOLD_PIN,LOW);
}

void batteryCheck(){
    batteryPercentage = M5.Power.getBatteryLevel();
}

//auto power save shutdown function
unsigned long lastActivityTime = 0;
unsigned long powerSaveDelay = 600000; //10 minutes
void activity(){ //scatter this about so it doesn't switch off when you're running an experiment
    lastActivityTime = millis();
}

void autoPowerSave(){
    if(batteryPercentage < 10){
        powerOff();
    }
    // turn off after x minutes of inactivity
    timeKeeper(lastActivityTime, powerSaveDelay, powerOff);
}


void startStopButton(){
    if(!menuOpen){
        if(M5.BtnA.wasPressed()) {
        M5.Display.setBrightness(DISPLAY_BRIGHTNESS);
        activity();
        playPause();
        scale.tare();
        }
    
    //long hold to reset the timer
        if(M5.BtnA.pressedFor(3000)){
            reset();
            activity();
        }
    }
}


//function to blank the screen pressing button A
static bool isScreenBlank = false;
void blankScreen(){
    if(M5.BtnC.wasPressed()){
        activity();
        if(isScreenBlank){
             M5.Display.setBrightness(DISPLAY_BRIGHTNESS);
        } else {
            M5.Display.setBrightness(0);
        }
        isScreenBlank = !isScreenBlank;
    }

    if(M5.BtnC.wasPressed()){
        M5.Display.setBrightness(DISPLAY_BRIGHTNESS);
    }
}

//function to cycle through sensorID[1] when button C is pressed
//need to update this so that it works for other attached sensors
void menuUpdate(){
if(M5.BtnB.wasPressed()){ //open and cycle the menu
    if(!menuOpen){ //begin the menu
        menuOpen = true; //blocks button A playpause
        menuScreen = 0; //root
        menuSelection = 0; //first item highlighted
        menuDraw(); 
    }
    else{ //cycle the menu
        menuSelection++;
        if(menuSelection == menuSelectionLength[menuScreen]){ //cycle back to the beginning of the menu
            menuSelection = 0;
            menuSelectionDelta = 0;
            
        } else if(menuSelection - menuSelectionDelta == menuSelectionDisplayMax){ //cycle the menu down
            menuSelectionDelta++;
        }
    
        menuDraw();
    }
}

if(M5.BtnA.wasPressed()){ //action based on the current screen and selection
    
if(menuOpen){  //it will call playpause if not
    activity();
    switch(menuScreen){
        case 0: //root
            if(menuSelection == 3){ //exit
                menuScreen = 0;
                menuSelection = 0;
                menuSelectionDelta = 0;
                menuOpen = false;
                displayFree = true;
                DisplayUpdate();
            }
            else if(menuSelection == 0){ //sensor
                if(SENSOR::sensorID[0] != 0){
                   menuChangeDraw("External","sensor","attached"); 
                   break;    
                }
                else {
                    menuScreen = 1;
                    menuSelection = 0;
                    menuSelectionDelta = 0;
                    menuDraw();
                }
            }
            else {    
                menuScreen = menuSelection + 1;
                menuSelection = 0;
                menuSelectionDelta = 0;
                menuDraw();
            }
            break;
        case 1: //sensor select (only displayed for internal sensors)
            if(menuSelection == 7){ //exit up one level
                menuScreen = 0;
                menuSelection = 0;
                menuSelectionDelta = 0;
                menuDraw();
            }
            else{
                //set the sensor ID
            SENSOR::sensorID[1] = menuSelection;
            Serial.print("Sensor ID: "); Serial.println(SENSOR::sensorID[1]);
            SENSOR::sensorDetect();
            SENSOR::sensorInit();
            reset();
            
            //display a message
            menuChangeDraw(SENSOR::sensorDetails[SENSOR::sensorID[0]][SENSOR::sensorID[1]][0],"selected","");

            //reset the menu
            menuScreen = 0;
            menuSelection = 0;
            menuSelectionDelta = 0;
            
            //exit now
            menuOpen = false;
            displayFree = true;
            DisplayUpdate();
            
            }   
            break;
        case 2: //interval
            if(menuSelection == 7){ //exit up one level
                menuScreen = 0;
                menuSelection = 0;
                menuSelectionDelta = 0;
                menuDraw();
            }
            else{
                if(burstMode){
                    burstInterval = String(menuTree[2][menuSelection]).toInt();  
                    sensorInterval = burstInterval; 
                }
                else{
                    sensorInterval = String(menuTree[2][menuSelection]).toInt();   
                    if(ExperimentInterval < sensorInterval){ // don't report data if it hasn't been remeasured
                        ExperimentInterval = sensorInterval;
                    }
                }
                menuChangeDraw("Interval =",menuTree[2][menuSelection],"milliseconds");
                 //reset the menu
                menuScreen = 0;
                menuSelection = 0;
                menuSelectionDelta = 0;
            
                //exit now
                menuOpen = false;
                displayFree = true;
                DisplayUpdate();
            }
           
            break;
        case 3: //loud or not loud
            if(menuSelection == 2){ //exit up one level
                menuScreen = 0;
                menuSelection = 0;
                menuSelectionDelta = 0;
                menuDraw();
            }
            else{
                if(menuSelection == 0){
                    loud = true;
                    menuChangeDraw("Loud","beeps","enabled");
                }
                else{
                    loud = false;
                    menuChangeDraw("Loud","beeps","disabled");
                }
                menuDraw();
            }
            break;
    }
}
}

}

