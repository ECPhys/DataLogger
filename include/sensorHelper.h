#pragma once


// Declare OneWire object outside the switch statement
OneWire oneWire(33);
//define one-wire sensor
DallasTemperature sensors(&oneWire);

//forward declarations
int burstCounter;
void callibrationRoutine();

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
    

    const char* sensorDetails[4][5][4] = {
        //sensorID (type of connection), sensor (list of different types of sensor within the ID group), sensorPROPERTIES {Name,Quantity,Unit,Conversion} 
        //ID = 0, internal sensor
        {
            //microphone
            {"Microphone","SPL","dB","1.0"},
            //accelerometer
            {"Accelerometer Raw","Acceleration","m/s^2","9.81"},
            //Net acceleration
            {"Net Acceleration","Acceleration","m/s^2","9.81"},
            //gyroscope
            {"Gyroscope","Angular Rotation","rad/s","1.0"},
            //magnotometer
             {"Magnetometer","B-Field","T","1.0"},
        },
        //ID = 1, One-wire sensors
        {
            //DS18B20
            {"DS18B20","Temperature","Deg C","1.0"},
            //DS18S20
            {"DS18S20","T","deg C","1.0"},
            //DS1822
            {"DS1822","T","C","1.0"},
            //DS18B20
            {"DS18B20","T","C","1.0"},
            //DS18B20
            {"DS18B20","T","C","1.0"}
        },
        //ID = 2, I2C sensors
        {
            //BMP180
            {"BMP180","Pressure","Pa","1.0"},
            //BMP280
            {"BMP280","P","Pa","1.0"},
            //BME280
            {"BME280","P","Pa","1.0"},
            //BMP180
            {"BMP180","P","Pa","1.0"},
            //BMP180
            {"BMP180","P","Pa","1.0"}
        },
        //ID = 3, Analog sensors
        {
            //NTC thermistor
            {"NTC Thermistor","Temperature","C","1.0"},
            //voltmeter
            {"Voltmeter","Voltage","V","1.0"},
            //ammeter
            {"Ammeter","Current","A","1.0"},
            //other
            {"Other","O","O","1.0"},
            //other
            {"Other","O","O","1.0"}
        }
    };

    int numberOfDevices = 0;

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
                    sensorID[1]=2; numberOfDevices = 1;//this might be changed. Default to acceleration net
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
                    sensorInterval = 20; //millis
                    ExperimentInterval = 20; //millis (20 milliseconds seems to be the fastest for acceleration passed to the website. still overloads after 10s)
                }
                else if (sensorID[1] == 2){//report just the magnitude of the acceleration
                    M5.Imu.begin();
                    Serial.println("IMU initialised");
                    callibrateAcc();
                    Serial.println("Acc callibrated");
                    burstMode = true;
                    sensorInterval = burstInterval; //millis
                    ExperimentInterval = burstReportInterval; //millis
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
       conversionFactor = atof(sensorDetails[SENSOR::sensorID[0]][SENSOR::sensorID[1]][3]);
    }

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
                    sensorReadings[3] = sqrt(x*x + y*y + z*z); //magnitude
                    sensorReadings[4] = sqrt(sensorReadings[3]*sensorReadings[3] - 1); //remove gravity )
                }
                if(sensorID[1] == 2) {//accelerometer, just the magnitude
                    float x,y,z,m2;
                    M5.Imu.getAccelData(&x, &y, &z);
                    x = x - callibrationDelta[0];
                    y = y - callibrationDelta[1];
                    z = z - callibrationDelta[2];
                    m2 = sqrt(x*x + y*y + z*z); //magnitude
                    sensorReadings[0] = m2; 
                    //burst mode!
                    if(running){
                    burstData[burstCounter][0] = sensorReadings[0];
                    Serial.print(sensorReadings[0]); Serial.print(",");  
                    burstCounter++;
                    if(fmod(burstCounter,100) == 0){
                        M5.Speaker.tone(2000, 100);
                    }
                    if(burstCounter == maxBurst){
                        playPause();
                        burstCounter = 0;
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
                break;
            case 3:
                //read the analog device
                break;
            default:
                break;
        }
    }

    void callibrateAcc(){
            callibrationRoutine();
                //calibrate the accelerometer to remove g
            
            float x,y,z;
            M5.Imu.getAccelData(&x, &y, &z);
            callibrationDelta[0] = x;
            callibrationDelta[1] = y;  
            callibrationDelta[2] = z;
            Serial.printf("Calibration: %f, %f, %f\n", 
            callibrationDelta[0]*9.81, 
            callibrationDelta[1]*9.81, 
            callibrationDelta[2]*9.81);
            
           
            delay(500);
            
            //M5.Lcd.fillScreen(TFT_BLACK);
                  
    }
}