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
    if(M5.BtnA.wasPressed()) {
    M5.Display.setBrightness(DISPLAY_BRIGHTNESS);
    activity();
    playPause();

    }

 //long hold to reset the timer
    if(M5.BtnA.pressedFor(3000)){
        reset();
        activity();
    }
}


//function to blank the screen pressing button b
static bool isScreenBlank = false;
void blankScreen(){
    if(M5.BtnB.wasPressed()){
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

