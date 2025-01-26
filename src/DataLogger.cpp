#include "globals.h"

void setup() {
    

    M5.begin();
    pinMode(POWER_HOLD_PIN,OUTPUT);digitalWrite(POWER_HOLD_PIN,HIGH);

    Serial.begin(115200);
    Serial.println("M5 initialized.");

    // Initialize the display
    DisplayInit();

    // Initialize the temperature sensor
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

    //This is the frequency of reporting the sensor data and updating the time and the display
    timeKeeper(lastExperimentUpdateTime, burstMode ? burstReportInterval : ExperimentInterval, experimentTimer);
    
    //Handle display updates happens in the experimentTimer function
    // Update BLE Characteristics happens in the experimentTimer function
    

    //Handle power management
    batteryCheck();
    autoPowerSave();
    
    //Handle changes in connected sensors

    
   
}