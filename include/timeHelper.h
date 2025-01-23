#pragma once


//Semi-constant variables for the time
unsigned long sensorInterval = 1000; //millis
unsigned long ExperimentInterval = 1000; //millis
unsigned long displayInterval = 1000; //millis

//Time variables for the sensor which may not update every time the experiment time is updated
unsigned long sensorLastUpdateTime = 0; //millis
unsigned long sensorStartTime = 0;  //millis

//Time variables for the display
unsigned long lastDisplayUpdateTime = 0; // millis
unsigned long displayTimeElapsed = 0; // millis

//Time variables for the experiment
unsigned long lastExperimentUpdateTime = 0; // millis
unsigned long experimentTimeElapsed = 0; // millis
unsigned long experimentStartTime = 0 ; // millis
unsigned long hours = 0;
unsigned long minutes = 0;
unsigned long seconds = 0;

//if the experiment is paused
unsigned long experimentPauseTime = 0; // millis
unsigned long experimentPauseStartTime = 0; // millis

//if the experiment is running
bool running = false;
bool paused = false;

//forward declaration of the display update function and BLE sensor update function
void DisplayUpdate();
void BLEUpdateSensorData();
void BLEUpdateMetadata();


//general function to keep track of time
void timeKeeper(unsigned long& lastUpdateTime,unsigned long& interval,void (*function)()){
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= interval) {
        lastUpdateTime = currentTime;
        function();
    }

}

//function to keep track of sensor time. This also updates the display and the bluetooth characteristics
void experimentTimer(){
    if(running){
        experimentTimeElapsed = millis() - experimentStartTime - experimentPauseTime;
        //eg. 15s = 41000 - 26000 - 0 
        //eg2 15s = 200,000 - 26,000 - 159,000
        hours = experimentTimeElapsed / 3600000;
        minutes = (experimentTimeElapsed / 60000) % 60;
        seconds = (experimentTimeElapsed / 1000) % 60;
        BLEUpdateSensorData();
        //debug print
        Serial.printf("Time: %02lu:%02lu:%02lu\n", hours, minutes, seconds);
    }
    else{
        if(experimentTimeElapsed != 0){
        experimentPauseTime = millis() - experimentPauseStartTime;
        //eg 2. Paused at 10,000, 
        }
        else{
            experimentPauseTime = 0;
        }
    }
    DisplayUpdate();
}

//playPause function
void playPause(){
    if(running){//pause
        //this pauses the experiment to be resumed later
        experimentPauseStartTime = millis();
        paused = true;
        running = false;
        Serial.println("Timer paused");
    }
    else{ //play
        if(!paused){
            //this starts the experiment from the beginning
            experimentStartTime = millis();
            experimentPauseTime = 0;
            Serial.println("Timer started");
        }
        else{
            //this resumes the experiment from the pause point
            experimentPauseTime = millis() - experimentPauseStartTime;
            Serial.println("Timer restarted");
        }
        paused = false;
        running = true;
    }
    BLEUpdateMetadata();
}

//reset function
void reset(){
    running = false;
    paused = false;
    BLEUpdateMetadata();
    Serial.println("Timer reset to 0");
}

