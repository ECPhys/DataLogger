#pragma once


// Declare OneWire object outside the switch statement
OneWire oneWire(33);
//define one-wire sensor
DallasTemperature sensors(&oneWire);

//forward declarations
int burstCounter;
void callibrationRoutine();
void buildMenuTree();

//implement madwick filter
//#include "MadgwickAHRS.h"
//Madgwick filter;


namespace SENSOR{

    float sensorReadings[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    float conversionFactor = 1.0;
    float callibrationDelta[3] = {0.0, 0.0, 0.0};

    bool sensorConnected = false;
    int sensorID[2] = {0,0}; 
    /*
    First element is the sensor ID
    Second element is the sensor
    
    0,0 = microphone
    0,1 = accelerometer
    0,2 = net acceleration 
    0,3 = gyroscope
    0,4 = magnetometer

    1,0 = DS18B20
    1,1 = DS18S20
    1,2 = DS1822
    1,3 = DS18B20
    1,4 = DS18B20

    2,0 = BMP180
    2,1 = BMP280
    2,2 = BME280
    2,3 = BMP180
    2,4 = BMP180

    3,0 = NTC thermistor
    3,1 = voltmeter
    3,2 = ammeter
    3,3 = other
    3,4 = other
      
    */
    int maxSensorsPerID = 7;

    const char* sensorDetails[4][7][6] = {
        //sensorID (type of connection), sensor (list of different types of sensor within the ID group), sensorPROPERTIES {0: Sensor Name,  1: Quantity / sensor labels,  2: Unit,  3: Conversion factor,  4: number of devices} 
        //ID = 0, internal sensor
        {
            //microphone [0]
            {"Microphone","Sound Pressure Level/SPL","dB","1.0","1"},
            //accelerometer [1]
            {"Accelerometer Raw","Acceleration/x/y/z/Magnitude/mag - g","m/s^2","9.81","3"},
            //Net acceleration [2]
            {"Y Acceleration","Acceleration/a (Y)","m/s^2","9.81","1"},
            //velocity [3]
            {"Velocity","Velocity/v (Y)","m/s","1.0","1"},
            //displacement [4]
            {"Displacement","Displacement/s (Y)","m","1.0","1"},
            //gyroscope [5]
            {"Gyroscope","Angular Rotation/x/y/z/magnitude","rad/s","1.0","3"},
            //magnotometer [6]
            {"Magnetometer","B-Field/x/y/z/magnitude","T","1.0","3"},
        },
        //ID = 1, One-wire sensors
        {
            //DS18B20 [0]
            {"DS18B20","Temperature/T1/T2/T3/T4/T5","Deg C","1.0","1"},
            //DS18S20
            {"DS18S20","T","deg C","1.0","1"},
            //DS1822
            {"DS1822","T","C","1.0","1"},
            //DS18B20
            {"DS18B20","T","C","1.0","1"},
            //DS18B20
            {"DS18B20","T","C","1.0","1"},
            //empty
            {"","","","","1"},
            //empty
            {"","","","","1"}
        },
        //ID = 2, I2C sensors
        {
            //BMP180
            {"BMP180","Pressure","Pa","1.0","1"},
            //BMP280
            {"BMP280","P","Pa","1.0","1"},
            //BME280
            {"BME280","P","Pa","1.0","1"},
            //BMP180
            {"BMP180","P","Pa","1.0","1"},
            //BMP180
            {"BMP180","P","Pa","1.0","1"},
            //empty
            {"","","","","1"},
            //empty
            {"","","","","1"}
        },
        //ID = 3, Analog sensors
        {
            //NTC thermistor
            {"NTC Thermistor","Temperature/T1/T2/T3/T4/T5","C","1.0","1"},
            //voltmeter
            {"Voltmeter","Voltage","V","1.0","1"},
            //ammeter
            {"Ammeter","Current","A","1.0","1"},
            //other
            {"Other","O","O","1.0","1"},
            //other
            {"Other","O","O","1.0","1"},
            //empty
            {"","","","","1"},
            //empty
            {"","","","","1"}
        }
    };

    int numberOfDevices = 0;

    void burstUpdate(){
        burstCounter++;
        if(fmod(burstCounter,100) == 0){
            if(loud){M5.Speaker.tone(2000, 100);}
        }
        if(burstCounter == maxBurst){
            burstCounter = 0;
            displayFree = true; //unblocks the display update
            //DisplayUpdate();
            Serial.println("Updating the display");
            playPause();
            experimentTimeElapsed = 0;
        }
    
    }

    void sensorDetect(){

        enum SensorState {
            CHECK_ONEWIRE,
            CHECK_I2C,
            CHECK_ANALOG,
            INTERNAL,
            DONE
        };

        SensorState state = CHECK_ONEWIRE;

        

        while(state != DONE){
            switch(state){
                case CHECK_ONEWIRE:
                    //Initialise the onewire bus and see if any sensors are connected.
                    Serial.println("Checking for onewire devices");
                    //begin with a 3v3 pullup on pin 32
                    pinMode(32,OUTPUT);
                    digitalWrite(32,HIGH);
                    //Initialise the onewire bus
                    
                    sensors.begin();
                    //count how many sensors are attached and report this
                    numberOfDevices = sensors.getDeviceCount();
                        if (numberOfDevices > 0){
                            sensorID[0] = 1; //one-wire sensor
                            sensorID[1] = 0; //DS18B20
                            sensorConnected = true;
                            Serial.print("onewire device found: ");
                            Serial.println(numberOfDevices);
                            state = DONE;
                        }
                        else{
                            Serial.println("No onewire devices found");
                            digitalWrite(32,LOW);
                            pinMode(32,INPUT);
                            oneWire.reset();
                            pinMode(33,INPUT);
                            state = CHECK_I2C;
                        }
                    break;

                case CHECK_I2C:
                    //Initialise the I2C bus and see if any sensors are connected.
                    Serial.println("Checking for I2C devices");
                    Wire.begin(32,33);
                    Wire.beginTransmission(0x00);
                    if (Wire.endTransmission() == 0){
                        sensorID[0] = 2;
                        sensorID[1] = 0; //BMP180
                        numberOfDevices = atoi(sensorDetails[sensorID[0]][sensorID[1]][4]);
                        sensorConnected = true;
                        Serial.println("I2C device found");
                        state = DONE;
                    }
                    else{
                        Serial.println("No I2C devices found");
                        state = CHECK_ANALOG;
                    }
                    break;

                case CHECK_ANALOG:
                    //Initialise the analog pin and see if any sensors are connected.
                    Serial.println("Checking for analog devices");
                    pinMode(36,INPUT);
                    if (analogRead(36) > 1000){    //needs updating to the correct details as and when
                        sensorID[0] = 3;
                        sensorID[1] = 0; //NTC thermistor
                        numberOfDevices = atoi(sensorDetails[sensorID[0]][sensorID[1]][4]);
                        sensorConnected = true;
                        Serial.println("Analog device found");
                        state = DONE;
                    }
                    else{
                        Serial.println("No analog devices found");
                        state = INTERNAL;
                    }
                    break;

                case INTERNAL:
                    Serial.println("internal sensors only");
                    sensorID[0]=0; //nothing connected - can still use internal sensors.
                    //put some control logic in here using the buttons
                    //sensorID[1]=3; //we are choosing here
                    numberOfDevices = atoi(sensorDetails[sensorID[0]][sensorID[1]][4]);
                    sensorConnected = false;                   
                    state = DONE;
                    //end of the state machine
                    break;
                case DONE:
                    break;
            }
        }
        
    }

    void sensorInit(){
        
       switch(sensorID[0]){
            case 0:
                //no device connected - using internal sensors
                //initiate the accelerometer
                if(sensorID[1] == 1) { //report all accelerometer data
                    M5.Imu.begin();
                    Serial.println("IMU initialised");
                    burstMode = false;
                    sensorInterval = 4; //millis -- allows for rolling average of 5 samples in 20 ms
                    ExperimentInterval = 40; //millis (20 milliseconds seems to be the fastest for acceleration passed to the website. Too fast for the display)
                }
                else if (sensorID[1] == 2){//report just the magnitude of the acceleration in the y direction
                    M5.Imu.begin();
                    Serial.println("IMU initialised");
                    //callibrateAcc();
                    Serial.println("Acc callibrated");
                    burstMode = true;
                    sensorInterval = burstInterval; //millis
                    //ExperimentInterval = burstReportInterval; //millis
                }
                else if (sensorID[1] == 3){//report the velocity
                    M5.Imu.begin();
                    Serial.println("IMU initialised");
                    //callibrateAcc();
                    Serial.println("Acc callibrated");
                    burstMode = true;
                    sensorInterval = burstInterval; //millis
                    //ExperimentInterval = burstReportInterval; //millis
                }
                else if (sensorID[1] == 4){//report the displacement
                    M5.Imu.begin();
                    Serial.println("IMU initialised");
                    //callibrateAcc();
                    Serial.println("Acc callibrated");
                    burstMode = true;
                    sensorInterval = burstInterval; //millis
                    //ExperimentInterval = burstReportInterval; //millis
                }
                else{
                    burstMode = false;
                    sensorInterval = 1000; //millis
                    ExperimentInterval = 1000; //millis
                }
                
                
                break;
            case 1:
                //request the first temperature reading
                sensors.requestTemperatures();
                burstMode = false;
                sensorInterval = 1000; //millis
                ExperimentInterval = 1000; 
                break;
            case 2:
                //initialise the I2C device
                break;
            case 3:
                //initialise the analog device
                break;
            default:
                break;
            
       }
        //DisplayUpdate();
        Serial.println("Sensors Initialised");
        Serial.print("Sensor ID: "); Serial.println(sensorID[1]);
        Serial.print("Number of devices: "); Serial.println(numberOfDevices);
        Serial.print("Sensor Interval: "); Serial.println(sensorInterval);
        Serial.print("Experiment Interval: "); Serial.println(ExperimentInterval);
        Serial.print("Burst Mode: "); Serial.println(burstMode);
       conversionFactor = atof(sensorDetails[SENSOR::sensorID[0]][SENSOR::sensorID[1]][3]);
    }
    
    const int DBufferLength = 25;
    float Dbuffer[DBufferLength] = {0.0};
   
    int Dcounter = 0;

    void sensorRead(){
        switch(sensorID[0]){
            case 0:
                //no device connected
                if(sensorID[1] == 1) {//accelerometer, all data readings scaled to g =1 
                    float x,y,z;
                    M5.Imu.getAccelData(&x, &y, &z);
                    sensorReadings[0] = x;
                    sensorReadings[1] = y;
                    sensorReadings[2] = z;
                    //sensorReadings[3] = sqrt(x*x + y*y + z*z); //magnitude
                    //sensorReadings[4] = sqrt(sensorReadings[3]*sensorReadings[3] - 1); //remove gravity )
                }
                if(sensorID[1] == 2) {//accelerometer, just the magnitude in the y direction
                    float x,y,z,m2;
                    
                    M5.Imu.getAccelData(&x, &y, &z);
                    //x = x - callibrationDelta[0];
                    y = y - callibrationDelta[1];
                    //z = z - callibrationDelta[2];
                    //m2 = sqrt(x*x + y*y + z*z); //magnitude
                    //rolling average in y
                    if (Dcounter < DBufferLength) {
                        Dbuffer[Dcounter] = y;
                    }
                    Dcounter++;
                    if(Dcounter == DBufferLength){
                        Dcounter = 0;  
                    }
                    
                    float sum = 0.0;
                    for (int i = 0; i < DBufferLength; i++) {
                        sum += Dbuffer[i];
                    }
                    sensorReadings[0] = sum / DBufferLength;
                    //sensorReadings[0] = y; //y value magnitude
                    //burst mode!
                    if(running){
                        burstData[burstCounter][0] = sensorReadings[0];
                        //Serial.print(sensorReadings[0]); Serial.print(",");  
                        burstUpdate();
                    }
                }
                if(sensorID[1] == 3) {// velocity in y direction
                    float x,z,y;
                    M5.Imu.getAccelData(&x, &y, &z);
                    y = y - callibrationDelta[1]; //y acceleration
                    //x = x - callibrationDelta[0]; //x acceleration
                    //z = z - callibrationDelta[2]; //z acceleration
                    //burst mode!
                    if(running){
                        if(burstCounter > 0){//calculate velocity in the y direction
                            burstData[burstCounter][1] = y; //store the acceleration in an unreported part of the array
                            burstData[burstCounter][0] = burstData[burstCounter-1][0]+9.81*burstData[burstCounter][1]*(burstInterval/1000.0); //v = u + at
                        }
                        else{
                            burstData[0][0] = 0.0;
                            burstData[0][1] = 0.0;
                        }
                        burstUpdate(); // advances the counter and waits until the buffer is full.
                    }

                }
                if(sensorID[1] == 4) {// displacement in y direction
                    float x,z,y;
                    M5.Imu.getAccelData(&x, &y, &z);
                    y = y - callibrationDelta[1]; //y acceleration
                    //x = x - callibrationDelta[0]; //x acceleration
                    //z = z - callibrationDelta[2]; //z acceleration
                    
                    //burst mode!
                    if(running){
                        if(burstCounter > 0){//calculate displacement in the y direction
                            burstData[burstCounter][1] = y; //store the acceleration in an unreported part of the array
                            burstData[burstCounter][2] = burstData[burstCounter-1][2]+9.81*burstData[burstCounter][1]*(burstInterval/1000.0); //v = u + at, stored in another unused part of the burst array
                            burstData[burstCounter][0] = burstData[burstCounter-1][0]+burstData[burstCounter-1][2]*(burstInterval/1000.0)+(burstData[burstCounter][1]*(burstInterval/1000.0)*(burstInterval/1000.0))/2; //s = s + ut + 0.5at^2
                        }
                        else{
                            burstData[0][0] = 0.0;
                            burstData[0][1] = 0.0;
                            burstData[0][2] = 0.0;
                        }
                        //Serial.print(sensorReadings[0]); Serial.print(",");  
                        burstUpdate();
                    }
                }
                
                break;
            
            case 1:
                //read the temperature
                for (int i = 0; i < numberOfDevices; i++) {
                    sensorReadings[i] = sensors.getTempCByIndex(i);
                }
                sensors.requestTemperatures();
                break;
            case 2:
                //read the I2C device
                break;
            case 3:
                //read the analog device
                break;
            default:
                break;
        }
        //DisplayUpdate(); // too often in burst mode
        
        //update the menu array dynamically
        buildMenuTree();
    }

    void callibrateAcc(){
            //calibrate the accelerometer to remove g
            float sensorBuffer[3][5] = {{0.0, 0.0, 0.0, 0.0, 0.0},{0.0, 0.0, 0.0, 0.0, 0.0},{0.0, 0.0, 0.0, 0.0, 0.0}};
            float x,y,z;
            for(int i = 0; i < 5; i++){
                M5.Imu.getAccelData(&x, &y, &z);
                sensorBuffer[0][i] = x;
                sensorBuffer[1][i] = y;
                sensorBuffer[2][i] = z;
                delay(5);
            }

            M5.Imu.getAccelData(&x, &y, &z);
            callibrationDelta[0] = (sensorBuffer[0][0] + sensorBuffer[0][1] + sensorBuffer[0][2] + sensorBuffer[0][3] + sensorBuffer[0][4])/5;
            callibrationDelta[1] = (sensorBuffer[1][0] + sensorBuffer[1][1] + sensorBuffer[1][2] + sensorBuffer[1][3] + sensorBuffer[1][4])/5;  
            callibrationDelta[2] = (sensorBuffer[2][0] + sensorBuffer[2][1] + sensorBuffer[2][2] + sensorBuffer[2][3] + sensorBuffer[2][4])/5;
            Serial.printf("Calibration: %f, %f, %f\n", 
            callibrationDelta[0]*9.81, 
            callibrationDelta[1]*9.81, 
            callibrationDelta[2]*9.81);
            
           
            
            //DisplayUpdate();
            //M5.Lcd.fillScreen(TFT_BLACK);
                  
    }
}