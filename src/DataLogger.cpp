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
    GLOBALS::startStopButton();
    //Handle control commands over BLE

    //This is the frequency of updating the sensor data
    timeKeeper(sensorLastUpdateTime, sensorInterval, SENSOR::sensorRead);

    //This is the frequency of reporting the sensor data and updating the time
    timeKeeper(lastExperimentUpdateTime, ExperimentInterval, experimentTimer);

    //Handle display updates happens in the experimentTimer function
    // Update BLE Characteristics happens in the experimentTimer function
    

    //Handle power management
    GLOBALS::batteryCheck();
    
    //Handle changes in connected sensors

    
   
}