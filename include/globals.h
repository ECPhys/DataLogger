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


namespace GLOBALS{

int batteryPercentage = 0;
void batteryCheck(){
    batteryPercentage = M5.Power.getBatteryLevel();
}

void startStopButton(){
    if(M5.BtnA.wasPressed()) {
    
    playPause();

    }

 //long hold to reset the timer
    if(M5.BtnA.pressedFor(3000)){
        reset();
    }
}


//function to blank the screen pressing button b
void blankScreen(){
    static bool isScreenBlank = false;
    if(M5.BtnB.wasPressed()){
        if(isScreenBlank){
            M5.Lcd.fillScreen(TFT_WHITE); // Assuming white is the default screen color
        } else {
            M5.Lcd.fillScreen(TFT_BLACK);
        }
        isScreenBlank = !isScreenBlank;
    }
}

}