#pragma once

// BLE Service UUID for the overall GATT service
#define SERVICE_UUID "1e85414a-a5f0-4976-92a1-bce08f690857"

//BLE characterisitic UUID for the sensor reading and timestamp 
#define SENSOR_CHARACTERISTIC_UUID "7a5a3d88-e7d4-49f6-a6ac-2f1579c93c80"
#define TIME_CHARACTERISTIC_UUID "22671dbe-1ce2-4f41-959a-8ccb81fdd7f7"

//BLE characteristic UUID for controlling the device
#define CONTROL_CHARACTERISTIC_UUID "25a8bcb3-a049-4d90-9a74-7a51767265f8"
//BLE characteristic for metadata about the device and sensors
#define METADATA_CHARACTERISTIC_UUID "96c1083c-dcca-4758-85d4-56f4b43092b4"


// BLE Server and Characteristics
BLEServer *pServer = NULL;
BLECharacteristic *pSensorCharacteristic = NULL;
BLECharacteristic *pTimeCharacteristic = NULL; // should deprecate this and put time into the first element of the sensor characteristic array
BLECharacteristic *pControlCharacteristic = NULL;
BLECharacteristic *pMetadataCharacteristic = NULL;

// Forward declaration of BLEUpdateMetadata
void BLEUpdateMetadata();

bool deviceConnected = false;
bool oldDeviceConnected = false;

//callback routines for the BLE server
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};



void onControlCharacteristicWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    Serial.print("Control characteristic written: ");
    Serial.println(value.c_str());

    int intValue = value[0]; // read the first element
    Serial.print("Control value parsed: ");
    Serial.println(intValue);

    if (intValue == 0) {
        reset();
        
    }
    if(intValue == 1){
        playPause();
        
    }
    
}

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pControlCharacteristic) {
    onControlCharacteristicWrite(pControlCharacteristic);
  }
};

// Initialize BLE
void BLEInit(){

// Initialize BLE
    BLEDevice::init("Datalogger");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create SENSOR BLE Characteristics
    pSensorCharacteristic = pService->createCharacteristic(
        SENSOR_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE
    );
    pSensorCharacteristic->addDescriptor(new BLE2902());

    // Create TIME BLE Characteristics
    pTimeCharacteristic = pService->createCharacteristic(
        TIME_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE
    );
    pTimeCharacteristic->addDescriptor(new BLE2902());

    // Create METADATA BLE Characteristics
    pMetadataCharacteristic = pService->createCharacteristic(
        METADATA_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE
    );
    pMetadataCharacteristic->addDescriptor(new BLE2902());

    // Create CONTROL BLE Characteristics
    pControlCharacteristic = pService->createCharacteristic(
        CONTROL_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE 
    );
    pControlCharacteristic->addDescriptor(new BLE2902());

    // Attach the callback function to the ControlCharacteristic
    pControlCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x00);  // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Waiting for a client connection to notify...");
}



//handle connections and disconnections
void BLEConnections(){
    if (!deviceConnected && oldDeviceConnected) {
    Serial.println("Device disconnected.");
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
    }

    if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    BLEUpdateMetadata();
    oldDeviceConnected = deviceConnected;
    Serial.println("Device Connected");
    }
}

//notify the BLE characteristics
void BLENotifyAll(){
    if(deviceConnected) {
        pSensorCharacteristic->notify();
        pTimeCharacteristic->notify();
        Serial.print("New value notified: ");
    }
}

//update the experiment metadata to the website
void BLEUpdateMetadata(){
    //Quantity, Unit, number of devices, sensor interval
    String x = "";
    x = String(SENSOR::sensorDetails[SENSOR::sensorID[0]][SENSOR::sensorID[1]][1]) + "," + String(SENSOR::sensorDetails[SENSOR::sensorID[0]][SENSOR::sensorID[1]][2]) + "," + String(SENSOR::numberOfDevices) + "," + String(sensorInterval);

    pMetadataCharacteristic->setValue((String(x)).c_str());
    pMetadataCharacteristic->notify();
}


//update the BLE characteristics
void BLEUpdateSensorData(){
    activity();
    //sensor string  millis elapsed (1h = 4B),(1B)sensor1 (4 bytes float),sensor2,sensor3, sensor4,sensor5 = 29 bytes
    //max packet length is 31 bytes
    String x = "";
    String m = ""; //millis since the start of the experiment
    m = String(experimentTimeElapsed);
    x = m + ","; //add the time to the start of the string
    
   
    for(int i = 0; i < SENSOR::numberOfDevices; i++){
         x += String(SENSOR::sensorReadings[i]*SENSOR::conversionFactor) + ",";
         Serial.println(x);
    }

    pSensorCharacteristic->setValue((String(x)).c_str());
    if(deviceConnected) {
        pSensorCharacteristic->notify();
    }
}
void toggleRunning(){
    //time string
    String m = String(experimentTimeElapsed);

    String r = ""; 
    if(running){
        r = "1";
    }
    else{
        r = "0";
    }
    
    String t = "";
    t = String(hours) + ":" + String(minutes < 10 ? "0" : "") + String(minutes) + ":" + String(seconds < 10 ? "0" : "") + String(seconds) ;

    pTimeCharacteristic->setValue((String(r) + "," + String(m) + "," + String(t)).c_str());
    if(deviceConnected) {
        pTimeCharacteristic->notify();
    }
}

//send the burst data over BLE
void BLESendBurstData(){
    //send the burst data over BLE
    for(int i = 0; i < maxBurst; i++){
        String x = "";
        int t = i*burstInterval;
        x = String(t) + ",";
        for(int j = 0; j < SENSOR::numberOfDevices; j++){
        x += String(burstData[i][j]) + ",";
        }
        pSensorCharacteristic->setValue((String(x)).c_str());
        
    if(deviceConnected) {
        pSensorCharacteristic->notify();
    }    
    delay(1); //delay to allow the BLE stack to send the data
    }
}