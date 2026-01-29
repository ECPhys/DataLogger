#pragma once


//Initialise the onewire bus
OneWire oneWire(33);
//define one-wire sensor
DallasTemperature sensors(&oneWire);

//create two IC2 objects for the two I2C buses

//TwoWire I2C1 = TwoWire(0);
//TwoWire I2C2 = TwoWire(1);


//forward declarations
int burstCounter;
void callibrationRoutine();
void buildMenuTree();

//implement madwick filter
//#include "MadgwickAHRS.h"
//Madgwick filter;

//NTC thermistor variables
const float V_REF = 3.3; //reference voltage
const float R1 = 7874.0; //resistor value
const int ADC_RESOLUTION = 4095; //ADC resolution
const float BETA = 5560.0; //beta value
const float R0 = 10000.0; //resistor value at 25 degrees C
const float T0 = 298.15; //temperature at 25 degrees C

//TOF variable
VL53L0X TOFsensor;

//Ultrasonic variable
SONIC_IO SONICsensor;
int point      = 0;
int last_point = 0;

//Voltmeter variables
#define M5_UNIT_VMETER_I2C_ADDR             0x49
#define M5_UNIT_VMETER_EEPROM_I2C_ADDR      0x53
#define M5_UNIT_VMETER_PRESSURE_COEFFICIENT 0.015918958F

ADS1115 Vmeter;

float Vresolution         = 0.0;
float Vcalibration_factor = 0.0;

//scales variables
#define LOADCELL_DOUT_PIN 33
#define LOADCELL_SCK_PIN  32

HX711 scale;
char info[100];

namespace SENSOR{

    float sensorReadings[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
    float conversionFactor = 1.0;
    float callibrationDelta[3] = {0.0, 0.0, 0.0};

    bool sensorConnected = false;
    int sensorID[2] = {0,0}; 
    /*
    First element is the sensor ID
    Second element is the sensor
    
    INTERNAL
    0,0 = microphone
    0,1 = accelerometer
    0,2 = net acceleration 
    0,3 = gyroscope
    0,4 = magnetometer

    ONEWIRE
    1,0 = DS18B20
    1,1 = DS18S20
    1,2 = DS1822
    1,3 = DS18B20
    1,4 = DS18B20

    I2C
    2,0 = BMP180
    2,1 = NCIR hat
    2,2 = TOF hat
    2,3 = Voltmeter
    2,4 = BMP180

    ANALOG
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
            //NCIR hat
            {"NCIR Hat","IR Temperature/T","deg C","1.0","1"},
            //Time of Flight Hat
            {"ToF Hat","Distance/D","m","1.0","1"},
            //Voltmeter
            {"Voltmeter","Voltage/V","V","1.0","1"},
            //BMP180
            {"BMP180","P","Pa","1.0","1"},
            //empty
            {"","","","","1"},
            //empty
            {"","","","","1"}
        },
        //ID = 3, Analog sensors
        {
            //0: NTC thermistor
            {"NTC Thermistor","Temperature/T","deg C","1.0","1"},
            //1: voltmeter
            {"Voltmeter","Voltage","V","1.0","1"},
            //2: ammeter
            {"Ammeter","Current","A","1.0","1"},
            //3: Ultrasonic Distance
            {"Ultrasonic Distance","Distance/D","m","1.0","1"},
            //4: Scales
            {"Scales","Mass/m","g","1.0","1"},
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
        if(burstCounter >= maxBurst){
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
                    //Using pin 32 for power as using the 5V line caused the battery sensor to read random results (feeding 5V back into the 3.3V data line)
                    pinMode(32,OUTPUT);
                    digitalWrite(32,HIGH);

                   //Initialise the onewire bus and see if any sensors are connected.
                    Serial.println("Checking for onewire devices");
                   
                    //start the one-wire bus
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
                            //power down the power pin
                            digitalWrite(32,LOW);
                            pinMode(32,INPUT);
                            //disable the one-wire bus
                            oneWire.write(0x00); //write a 0 to the bus to disable it
                            //oneWire.reset();
                            pinMode(33,INPUT); //is this needed?
                            state = CHECK_I2C;
                        }
                    break;

                case CHECK_I2C:
                    
                    //Initialise the I2C bus and see if any sensors are connected.
                    Serial.println("Checking for I2C devices");
                    Serial.println("I2C1 bus");
                    //check the first I2C bus (grove port)

                    
                    Wire.begin(32,33);
                    Wire.beginTransmission(0x00);
                    Wire.write(0x07); //write to the device once to see if it is there
                    if (Wire.endTransmission() == 0){
                        sensorID[0] = 2;
                        sensorID[1] = 0; //BMP180
                        numberOfDevices = atoi(sensorDetails[sensorID[0]][sensorID[1]][4]);
                        sensorConnected = true;
                        Serial.println("BMP180 device found on bus 1");
                        state = DONE;
                    }
                    else{
                        Serial.println("BMP180 not found on bus 1");
                        //Wire.end(); //end the previous I2C bus
                        //check for voltmeter
                        Wire.beginTransmission(M5_UNIT_VMETER_I2C_ADDR); //0x49
                        if(Wire.endTransmission() == 0){ //voltmeter is there
                            Serial.println("Voltmeter device found on bus 1. Initialising...");
                            Wire.end(); //use the library's implemetation of the I2C bus
                        
                        Vmeter.begin(&Wire, M5_UNIT_VMETER_I2C_ADDR, 32, 33, 400000U); 
                        Vmeter.setEEPROMAddr(M5_UNIT_VMETER_EEPROM_I2C_ADDR);
                        Vmeter.setMode(ADS1115_MODE_CONTINUOUS);
                        Vmeter.setRate(ADS1115_RATE_860);
                        Vmeter.setGain(ADS1115_PGA_512);
                        // | PGA      | Max Input Voltage(V) |
                        // | PGA_6144 |        128           |
                        // | PGA_4096 |        64            |
                        // | PGA_2048 |        32            |
                        // | PGA_512  |        16            | --< this is the default
                        // | PGA_256  |        8             |

                        Vresolution = Vmeter.getCoefficient() / M5_UNIT_VMETER_PRESSURE_COEFFICIENT;
                        Vcalibration_factor = Vmeter.getFactoryCalibration();
                        
                            sensorID[0] = 2;
                            sensorID[1] = 3; //voltmeter
                            numberOfDevices = atoi(sensorDetails[sensorID[0]][sensorID[1]][4]);
                            sensorConnected = true;
                            Serial.println("Initialisation complete");
                            state = DONE;
                            
                        }
                        else{ 
                            Serial.println("No I2C devices found on bus 1");
                            Wire.end(); //end the previous I2C bus
                            
                            
                            //check the 2nd I2C bus (Hats)
                            Serial.println("I2C2 bus");
                            Wire.begin(0,26);

                            //NCIR Hat
                            Wire.beginTransmission(0x5A);
                            Wire.write(0x07); //write to the device once to see if it is there
                            if (Wire.endTransmission() == 0){ // use Wire.requestFrom(0x5A, 2) and check this instead. 
                                sensorID[0] = 2;
                                sensorID[1] = 1; //NCIR Hat
                                numberOfDevices += atoi(sensorDetails[sensorID[0]][sensorID[1]][4]); //add the number of devices to the total
                                sensorConnected = true;
                                Serial.println("I2C device found on bus 2");
                                state = DONE;
                            }
                            else{
                                // check for ToF Hat
                                Wire.begin(0, 26);
                                                        
                                if(!TOFsensor.init()){
                                Serial.println("Failed to detect and initialize sensor!");
                                }
                                TOFsensor.setTimeout(500);
                                TOFsensor.startContinuous(20);
                                //Wire.beginTransmission(0x29);
                                //Wire.write(0x07); //write to the device once to see if it is there
                                if (Wire.endTransmission() == 0){
                                    sensorID[0] = 2;
                                    sensorID[1] = 2; //NCIR Hat
                                    numberOfDevices += atoi(sensorDetails[sensorID[0]][sensorID[1]][4]); //add the number of devices to the total
                                    sensorConnected = true;
                                    Serial.println("I2C device found on bus 2");
                                    state = DONE;
                                }
                                else{ //last case of nested if statements
                                    Serial.println("No I2C devices found on bus 2");
                                    Wire.end(); //end the previous I2C bus
                                    state = CHECK_ANALOG;
                                } 
                            }
                        }
                    }
                    break;

                case CHECK_ANALOG:
                    //Initialise the analog pin and see if any sensors are connected.
                    //check for Ultrasonic Range sensor
                    Serial.println("Checking for Scales kit");
                    SONICsensor.begin(32, 33);
                    delay(500);//let the pin settle
                    Serial.println(SONICsensor.getDistance());

                    if(fabs(SONICsensor.getDistance() - 13.60) < 0.001){ //this checks for the scales very well! returns 13.60 when the scales are connected
                       //cancel the ultrasonic sensor
                       Serial.println("Scales kit found");
                        pinMode(32,INPUT);
                        pinMode(33,INPUT);

                        // declare the scales
                        sensorID[0] = 3;
                        sensorID[1] = 4; //scales
                        numberOfDevices = atoi(sensorDetails[sensorID[0]][sensorID[1]][4]);
                        sensorConnected = true;
                        state = DONE;
                        
                    }
                    else if(SONICsensor.getDistance() > 0.1){ // 13.60 is what gets returned by the load cell when it is connected
                        sensorID[0] = 3;
                        sensorID[1] = 3; //ultrasonic
                        numberOfDevices = atoi(sensorDetails[sensorID[0]][sensorID[1]][4]);
                        sensorConnected = true;
                        Serial.println("Ultrasonic device found");
                        state = DONE;
                    }
                    else{
                        Serial.println("No ultrasonic devices found");
                        pinMode(32,INPUT);
                        pinMode(33,INPUT);

                        //NTC temp probe check
                        Serial.println("Checking for analog temperature probe");
                        pinMode(33,INPUT);
                        delay(500);//let the pin settle
                        if (analogRead(33) > 500){    //needs updating to the correct details as and when
                            sensorID[0] = 3;
                            sensorID[1] = 0; //NTC thermistor
                            numberOfDevices = atoi(sensorDetails[sensorID[0]][sensorID[1]][4]);
                            sensorConnected = true;
                            Serial.println("Analog Temp Probe found");
                            state = DONE;
                        }
                        else{   
                            //last case of nested if statements
                            Serial.println("No analog devices found");
                            state = INTERNAL;
                        }
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
        
       numberOfDevices = atoi(sensorDetails[sensorID[0]][sensorID[1]][4]);

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
                    
                }
                else if (sensorID[1] == 3){//report the velocity
                    M5.Imu.begin();
                    Serial.println("IMU initialised");
                    //callibrateAcc();
                    Serial.println("Acc callibrated");
                    burstMode = true;
                    sensorInterval = burstInterval; //millis
                    
                }
                else if (sensorID[1] == 4){//report the displacement
                    M5.Imu.begin();
                    Serial.println("IMU initialised");
                    //callibrateAcc();
                    Serial.println("Acc callibrated");
                    burstMode = true;
                    sensorInterval = burstInterval; //millis
                    
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
                if(sensorID[1] == 0) { //BMP180
                    //BMP180.begin(0x77); //address of the device
                    burstMode = false;
                    sensorInterval = 1000; //millis
                    ExperimentInterval = 1000; 
                }
                else if(sensorID[1] == 1) { //NCIR Hat
                    //NCIR.begin(0x5A); //address of the device
                    burstMode = false;
                    sensorInterval = 1000; //millis
                    ExperimentInterval = 1000; 
                }
                else if(sensorID[1] == 2) { //TOF Hat
                    //TOF.begin(0x29); //address of the device
                    burstMode = false;
                    sensorInterval = 100; //millis
                    ExperimentInterval = 100; 
                    
                }
                 else if(sensorID[1] == 3) { //Voltmeter
                    burstMode = false;
                    sensorInterval = 200; //millis (have changed this from standard constant reading)
                    ExperimentInterval = 200; 
                }
                else{
                    burstMode = false;
                    sensorInterval = 1000; //millis
                    ExperimentInterval = 1000; 
                }
                break;
            case 3:
                //initialise the analog device
                if(sensorID[1] == 0) { //NTC thermistor
                    burstMode = false;
                    sensorInterval = 1000; //millis
                    ExperimentInterval = 1000; 
                }
                else if(sensorID[1] == 1) { //voltmeter
                    burstMode = false;
                    sensorInterval = 1000; //millis
                    ExperimentInterval = 1000; 
                }
                else if(sensorID[1] == 2) { //ammeter
                    burstMode = false;
                    sensorInterval = 1000; //millis
                    ExperimentInterval = 1000; 
                }
                else if(sensorID[1] == 3) { //ultrasonic distance
                    burstMode = false;
                    sensorInterval = 100; //millis
                    ExperimentInterval = 100; 
                }
                else if(sensorID[1] == 4) { //scales
                    burstMode = false;
                    sensorInterval = 2000; //millis
                    ExperimentInterval = 2000; 
                   
                    //initialise the scales
                    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
                    // The scale value is the adc value corresponding to 1g
                    scale.set_scale(27.61f);  // set scale
                    scale.tare();             // auto set offset
                
                }
                else{
                    burstMode = false;
                    sensorInterval = 1000; //millis
                    ExperimentInterval = 1000; 
                }
                break;
            default:
                break;
            
       }
       //set the experiment interval based on the sensor interval
        ExperimentInterval = sensorInterval>20 ? sensorInterval : 20; //millis (20 milliseconds seems to be the fastest for acceleration passed to the website. Too fast for the display)

        //DisplayUpdate();
        Serial.println("Sensors Initialised");
        Serial.print("Sensor ID: "); Serial.println(sensorID[1]);
        Serial.print("Number of devices: "); Serial.println(numberOfDevices);
        Serial.print("Sensor Interval: "); Serial.println(sensorInterval);
        Serial.print("Experiment Interval: "); Serial.println(ExperimentInterval);
        Serial.print("Burst Mode: "); Serial.println(burstMode);
       conversionFactor = atof(sensorDetails[SENSOR::sensorID[0]][SENSOR::sensorID[1]][3]);

       //update the menu array dynamically
        buildMenuTree();
    }
    
    //used for smoothing averages
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
                    
                    //if burst mode
                    if(running && burstMode){
                        burstData[burstCounter][0] = sensorReadings[0];
                        burstData[burstCounter][1] = sensorReadings[1];
                        burstData[burstCounter][2] = sensorReadings[2];
                        //burstData[burstCounter][3] = sensorReadings[3];
                        //burstData[burstCounter][4] = sensorReadings[4];
                        burstUpdate();
                    }
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
                    if(running && burstMode){
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
                    sensorReadings[0] = y; //report the acceleration in y for information in continuous mode
                    if(running && burstMode){
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
                    float x,z,y,vy,s;
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
                            sensorReadings[0] = burstData[burstCounter][0]; //report the displacement in y for information in continuous mode
                        }
                        else{
                            burstData[0][0] = 0.0;
                            burstData[0][1] = 0.0;
                            burstData[0][2] = 0.0;
                        }
                        //Serial.print(sensorReadings[0]); Serial.print(",");  
                        if(burstMode){
                            burstUpdate(); // advances the counter and waits until the buffer is full. Putting this inside this check allows continuous mode
                        }
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
                if(sensorID[1] == 0) { //BMP180
                    //BMP180.readSensor();
                   
                }
                else if(sensorID[1] == 1) { //NCIR Hat
                    //NCIR.readSensor();
                    
                    Wire.beginTransmission(0x5A); //address of the device
                    Wire.write(0x07); //write to the device once to see if it is there
                    Wire.endTransmission(false); //send a repeated start
                    Wire.requestFrom(0x5A, 2); //request 2 bytes of data from the device
                    int rawData = Wire.read(); // Use an intermediate integer variable
                    rawData |= Wire.read() << 8; // Read the second byte of data and shift it to the left by 8 bits
                    sensorReadings[0] = rawData; // Assign the result back to sensorReadings[0]
                    sensorReadings[0] = sensorReadings[0] * 0.02 - 273.15; //convert the data to degrees C
                    
                }
                else if(sensorID[1] == 2) { //ToF Hat
                    uint16_t distance = TOFsensor.readRangeContinuousMillimeters();
                    if(TOFsensor.timeoutOccurred()) {
                        Serial.println("Sensor timeout!");
                    }
                    else {
                        if(distance > 2000){distance = 2000;}
                        sensorReadings[0] = round((distance / 1000.0) * 1000) / 1000.0; //distance in m, rounded to 3 decimal places
                        
                    }
                }
                else if(sensorID[1] == 3) { //Voltmeter
                    int16_t adc_raw = Vmeter.getSingleConversion(); 
                    sensorReadings[0]  = adc_raw * Vresolution * Vcalibration_factor / 1000.0; //convert to volts              
                    
                }
                break;

            case 3:
                //read the analog device
                if(sensorID[1] == 0) { //NTC thermistor
                    int rawData = analogRead(33); //read the analog pin
                    float vOut = (rawData * V_REF) / ADC_RESOLUTION; //convert to volts
                    float R2 = (R1 * vOut) / (V_REF - vOut); //calculate the resistance of the thermistor
                    float T = 1.0 / (log(R2 / R0) / BETA + 1.0 / T0) - 273.15; //calculate the temperature in degrees C
                    sensorReadings[0] = T; //store the temperature in degrees C

                }
                else if(sensorID[1] == 1) { //voltmeter
                    sensorReadings[0] = analogRead(33); //read the analog pin
                    sensorReadings[0] = (sensorReadings[0] * 3.3) / 4095.0; //convert to volts
                }
                else if(sensorID[1] == 2) { //ammeter
                    sensorReadings[0] = analogRead(33); //read the analog pin
                    sensorReadings[0] = (sensorReadings[0] * 3.3) / 4095.0; //convert to volts
                }
                else if(sensorID[1] == 3) { //Ultrasonic
                
                    sensorReadings[0] = SONICsensor.getDistance()/1000.0; //distance in m
                    if(sensorReadings[0] > 4.00){sensorReadings[0] = 4.00;}//4m max
                    if(sensorReadings[0] < 0.02){sensorReadings[0] = 0.02;}//2cm min
                }
                else if(sensorID[1] == 4) { //scales
                    sensorReadings[0] = scale.get_units(3); //get the weight in grams, 8 readings taken and averaged 
                    
                    
                }
                else{
                    Serial.println("No Analog device found");
                }
                break;
            default:
                break;
        }
        //DisplayUpdate(); // too often in burst mode

        //if burst mode (default 1 reading), update the burst data array. N/A for internals which have more complex handling
        if(SENSOR::sensorID[0] != 0){
            if(running && burstMode){
                        burstData[burstCounter][0] = sensorReadings[0];
                        burstUpdate();
                    }
         }
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