#include <M5Unified.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <queue>
#include <mutex>

// TEMPERATURE: Data wire is connected to GPIO33 (yellow wire)
#define ONE_WIRE_BUS 33

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

unsigned long lastUpdateTime = 0;
unsigned long startTime = 0;
const unsigned long updateInterval = 1000; 

//initialise temperature variable array for up to 5 sensors
float temperature[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
int numberOfDevices = 0;
//float temperatureC = 0.0;
int batteryPercentage = 0;
int batteryCurrent = 0;
int batteryPower = 0;

// BLE Service and Characteristic UUIDs
#define SERVICE_UUID "1e85414a-a5f0-4976-92a1-bce08f690857"
#define TEMP_CHARACTERISTIC_UUID "7a5a3d88-e7d4-49f6-a6ac-2f1579c93c80"
#define TIME_CHARACTERISTIC_UUID "22671dbe-1ce2-4f41-959a-8ccb81fdd7f7"

// BLE Server and Characteristics
BLEServer *pServer = NULL;
BLECharacteristic *pTempCharacteristic = NULL;
BLECharacteristic *pTimeCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

//callback routines
std::queue<std::string> timeCharacteristicQueue;
std::mutex queueMutex;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

//if the time characteristic is written to with a 0, reset the timer
/*void onTimeCharacteristicWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value == "0") {
        // Reset the timer
        startTime = millis();
        Serial.println("Timer reset to 0");
    }
}
//working version?
*/

void onTimeCharacteristicWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        timeCharacteristicQueue.push(value);
    }
    Serial.println("Write request added to queue");
    
}

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pTimeCharacteristic) {
    onTimeCharacteristicWrite(pTimeCharacteristic);
  }
};

void processTimeCharacteristicQueue() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (!timeCharacteristicQueue.empty()) {
        std::string value = timeCharacteristicQueue.front();
        timeCharacteristicQueue.pop();
        pTimeCharacteristic->setValue(value.c_str());
        pTimeCharacteristic->notify();
        Serial.println("Processed write request from queue");

        if (value == "0") {
        // Reset the timer
        startTime = millis();
        Serial.println("Timer reset to 0");
        }
    }
}

// Create a sprite canvas
M5Canvas canvas(&M5.Lcd);

void setup() {
    //pin to provide 3v3 pullup to the DS18B20 via the white pin of the grove connector
    pinMode(32,OUTPUT);
    digitalWrite(32,HIGH);

    M5.begin();
 
    M5.Display.setBrightness(2);
    M5.Display.setRotation(1);
    M5.Display.powerSaveOn();


    Serial.println("M5 initialized.");

    // Start the DS18B20 sensor
    sensors.begin();

    //count how many sensors are attached
    numberOfDevices = sensors.getDeviceCount();

     // Initialize BLE
    BLEDevice::init("TempLogger");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create TEMP BLE Characteristics
    pTempCharacteristic = pService->createCharacteristic(
        TEMP_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE
    );
    
    //create a BLE descriptor
    pTempCharacteristic->addDescriptor(new BLE2902());

    // Create TIME BLE Characteristics
    pTimeCharacteristic = pService->createCharacteristic(
        TIME_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE
    );
    
    //create a BLE descriptor
    pTimeCharacteristic->addDescriptor(new BLE2902());

     // Attach the callback function to the TimeCharacteristic
    pTimeCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x00);  // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Waiting for a client connection to notify...");

    // Record the start time
    startTime = millis();

    // Initialize the sprite canvas
    canvas.createSprite(240, 135); // Create a sprite with the same size as the screen

    // Request the initial temperature
    sensors.requestTemperatures();
}
    
void loop() {
  M5.update();
  if(M5.BtnA.wasPressed()) {
    startTime = millis();
    }

    unsigned long currentTime = millis();

    // Update the display every second
    if (currentTime - lastUpdateTime >= updateInterval) {
        lastUpdateTime = currentTime;

        // Calculate the elapsed time since start
        unsigned long elapsedTime = currentTime - startTime;
        unsigned long hours = elapsedTime / 3600000;
        unsigned long minutes = (elapsedTime % 3600000) / 60000;
        unsigned long seconds = (elapsedTime % 60000) / 1000;

        // Fetch the temperatures in Celsius for each sensor
        for (int i = 0; i < numberOfDevices; i++) {
            temperature[i] = sensors.getTempCByIndex(i);
        }
        
        // Fetch the battery
        batteryPower = M5.Power.getBatteryLevel();
        
        // Clear the sprite canvas
        canvas.fillSprite(BLACK);
        
        //change the battery percentage colour based on the value
        if (batteryPower > 50) {
            canvas.setTextColor(GREEN, BLACK);
        } else if (batteryPower > 20) {
            canvas.setTextColor(YELLOW, BLACK);
        } else {
            canvas.setTextColor(RED, BLACK);
        }

        // Set the font size for the battery percentage
        canvas.setTextSize(2);
        canvas.setCursor(238 - canvas.textWidth(String(batteryPower) + "%"), 2);
        canvas.print(String(batteryPower) + "%");

        

        // Set the font size for the timer and temperature
        canvas.setTextSize(3); // Increase the font size
        
        // Print the timer to the sprite canvas
        canvas.setCursor(0, 0);
        canvas.setTextColor(WHITE, BLACK);
        canvas.printf("%02lu:%02lu:%02lu", hours, minutes, seconds);
        
        // Print the temperature label to the sprite canvas
        
        
        for(    int i = 0; i < numberOfDevices; i++) {
            canvas.setCursor(10, 23+(i*23));
            canvas.setTextColor(TFT_CYAN);
            canvas.setTextSize(3);canvas.print("T");
            canvas.setTextSize(2);canvas.print(i);
            canvas.setTextSize(3);canvas.print(":");
            canvas.setTextSize(3);canvas.printf("%.1f", temperature[i]+1);
        }

        canvas.setTextColor(TFT_WHITE);
        // Print the temperature unit to the sprite canvas
        static const int degreesX = 200;
        static const int degreesY = 100;
        canvas.setTextSize(2);
        canvas.setCursor(degreesX, degreesY-10);
        canvas.printf("o");
        canvas.setTextSize(3);
        canvas.setCursor(degreesX+15, degreesY);
        canvas.printf("C");
        
        
        // Push the sprite to the screen
        canvas.pushSprite(0, 0);
        
        // Request the next temperature
        sensors.requestTemperatures();

        // Update BLE Characteristics
        pTempCharacteristic->setValue(String(temperature[0]).c_str());
        

        pTimeCharacteristic->setValue((std::to_string(hours) + " : " + std::to_string(minutes) + " : " + std::to_string(seconds)).c_str());

        if(deviceConnected) {
            pTempCharacteristic->notify();
            pTimeCharacteristic->notify();
            Serial.print("New value notified: ");
        }
    }

    // handle connections and disconnections
    if (!deviceConnected && oldDeviceConnected) {
    Serial.println("Device disconnected.");
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("Start advertising");
    oldDeviceConnected = deviceConnected;
    }
    //process the time characteristic queue
    processTimeCharacteristicQueue();
    
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
        Serial.println("Device Connected");
    }

}