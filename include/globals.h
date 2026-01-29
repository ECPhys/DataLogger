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
    static unsigned long lastBatCheckTime = 0;
    static const unsigned long BAT_CHECK_INTERVAL = 1000; // 1 second
    static const int BAT_WINDOW_SIZE = 30; // 30 second rolling average
    static int batteryReadings[BAT_WINDOW_SIZE] = {0};
    static int readIndex = 0;
    static unsigned long readCount = 0;
    
    unsigned long currentTime = millis();
    
    if(currentTime - lastBatCheckTime >= BAT_CHECK_INTERVAL){
        lastBatCheckTime = currentTime;
        
        // Get new reading
        batteryReadings[readIndex] = M5.Power.getBatteryLevel();
        readIndex = (readIndex + 1) % BAT_WINDOW_SIZE;
        readCount++;
        
        // Calculate average
        int sum = 0;
        int samples = (readCount < BAT_WINDOW_SIZE) ? readCount : BAT_WINDOW_SIZE;
        for(int i = 0; i < samples; i++){
            sum += batteryReadings[i];
        }
        batteryPercentage = sum / samples;
    }
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
            if(SENSOR::sensorID[0] == 3 && SENSOR::sensorID[1] == 4){ //if using the load cell, tare it when starting
                scale.tare();
            }
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
            if(menuSelection == 4){ //exit
                menuScreen = 0;
                menuSelection = 0;
                menuSelectionDelta = 0;
                menuOpen = false;
                displayFree = true;
                DisplayUpdate();
            }
            else if(menuSelection == 0){ //sensor change. Only allowed if internal sensors are being used. check this now
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
            else {    //iterate to the next menu screen
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
        case 2: //burst mode
            if(menuSelection == 2){ //exit up one level
                menuScreen = 0;
                menuSelection = 0;
                menuSelectionDelta = 0;
                menuDraw();
            }
            else{
                if(menuSelection == 0){
                    burstMode = true;
                    buildMenuTree(); //update the interval menu
                    sensorInterval = burstInterval; //ensure the sensor interval is set correctly
                    menuChangeDraw("Burst Mode","enabled","");
                }
                else if(menuSelection == 1){
                    burstMode = false;
                    buildMenuTree(); //update the interval menu
                    sensorInterval = 1000; //reset to default
                    ExperimentInterval = sensorInterval>20 ? sensorInterval : 20; //millis (20 milliseconds seems to be the fastest for acceleration passed to the website. Too fast for the display)

                    menuChangeDraw("Burst Mode","disabled","");
                }
                else{
                    menuDraw();
                }
            }
            break;
        
        
        case 3: //interval
            if(menuSelection == 7){ //exit up one level
                menuScreen = 0;
                menuSelection = 0;
                menuSelectionDelta = 0;
                menuDraw();
            }
            else{
                if(burstMode){
                    burstInterval = String(menuTree[3][menuSelection]).toInt();  
                    sensorInterval = burstInterval; 
                }
                else{
                    sensorInterval = String(menuTree[3][menuSelection]).toInt();   
                    if(ExperimentInterval < sensorInterval){ // don't report data if it hasn't been remeasured
                        ExperimentInterval = sensorInterval;
                    }
                }
                menuChangeDraw("Interval =",menuTree[3][menuSelection],"milliseconds");
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
        case 4: //loud or not loud
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

