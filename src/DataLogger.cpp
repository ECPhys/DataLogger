#include "globals.h"

void setup() {
    

    M5.begin();
    pinMode(POWER_HOLD_PIN,OUTPUT);digitalWrite(POWER_HOLD_PIN,HIGH);

    Serial.begin(115200);
    Serial.println("M5 initialized.");

    // Initialize the display
    DisplayInit();

    SENSOR::sensorID[1]=3; //we are choosing here which internal sensor to begin with

    // Autodetect which sensors are attached
    SENSOR::sensorDetect();
    //Initialise the sensors
    SENSOR::sensorInit();

    // Initialize BLE
    BLEInit();

    

}
    
void loop() {
    
    M5.update();  //need to call this every loop to check for button presses etc.

    BLEConnections(); // Handle BLE connections

    //Handle button presses
    startStopButton();
    blankScreen();
    //Handle control commands over BLE - this is handled directly in BlEHelper.h


    //This is the frequency of updating the sensor data
    timeKeeper(sensorLastUpdateTime, sensorInterval, SENSOR::sensorRead);

    //This is the frequency of reporting the sensor data to BLE and updating the time for the display.
    timeKeeper(lastExperimentUpdateTime, ExperimentInterval, experimentTimer); //burst mode has an alternate protocol

    //This is the frequency of updating the display
    timeKeeper(lastDisplayUpdateTime, displayInterval, DisplayUpdate);
    
    // Update BLE Characteristics happens in the experimentTimer function
    
    //Handle power management
    batteryCheck();
    autoPowerSave();
    
    //Handle changes in connected sensors
    menuUpdate();
    
   
}