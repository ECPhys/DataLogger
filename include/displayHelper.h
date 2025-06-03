#pragma once

//forward declaration of the battery percentage variable
    int batteryPercentage = 0;

// Create a sprite canvas
    M5Canvas canvas(&M5.Lcd);
    int DISPLAY_BRIGHTNESS = 2;
    bool defaultDisplay = false;
    
void DisplayInit(){
    // Set the brightness of the display
    M5.Display.setBrightness(DISPLAY_BRIGHTNESS);
    // Set the orientation of the display
    M5.Display.setRotation(1);

    //create a sprite canvas
    canvas.createSprite(240, 135);

    //Set the font
    canvas.loadFont("ArialMT_Plain_24");    
}   

void DisplayUpdate(){
    //Serial.println(displayFree);
if(displayFree){    //clear the canvas
        canvas.fillSprite(TFT_BLACK);
        // Set the font size for the timer and temperature
        canvas.setTextSize(3); // Increase the font size
        

        // Print the timer to the sprite canvas
        canvas.setCursor(0, 0);
        canvas.setTextColor(WHITE, BLACK);
        canvas.printf("%02lu:%02lu:%02lu", hours, minutes, seconds);
        
    defaultDisplay = true;
        // Print the sensor data to the sprite canvas
    switch (SENSOR::sensorID[0]) {
        case 0:
            canvas.setCursor(10, 23);
            if(SENSOR::sensorID[1] == 1){
                defaultDisplay = false;
                // Print the enitre accelerometer data to the sprite canvas
                canvas.setTextColor(TFT_CYAN);
                canvas.setTextSize(3);canvas.print("X:");
                canvas.setTextSize(3);canvas.printf("%.2f", SENSOR::sensorReadings[0]);
                
                canvas.setCursor(10, 46);
                canvas.setTextSize(3);canvas.print("Y:");
                canvas.setTextSize(3);canvas.printf("%.2f", SENSOR::sensorReadings[1]);
                
                canvas.setCursor(10, 69);
                canvas.setTextSize(3);canvas.print("Z:");
                canvas.setTextSize(3);canvas.printf("%.2f", SENSOR::sensorReadings[2]);
            
                canvas.setCursor(10, 92);
                canvas.setTextColor(TFT_ORANGE);
                canvas.setTextSize(3);canvas.print("m:");
                canvas.setTextSize(3);canvas.printf("%.2f", SENSOR::sensorReadings[3]);
                canvas.setCursor(10, 115);
            }
                   
            
            
            else if(SENSOR::sensorID[1] == 2){
                defaultDisplay = false;
                canvas.setTextColor(TFT_YELLOW);
                canvas.setTextSize(3);canvas.print("a:");
                canvas.setTextSize(3);canvas.printf("%.2f", SENSOR::sensorReadings[0]*SENSOR::conversionFactor);

                
                // Print the sensor unit to the sprite canvas
                canvas.setTextColor(TFT_WHITE);
                static const int UnitX = 170;
                static const int UnitY = 100;
                canvas.setTextSize(3);
                canvas.setCursor(UnitX, UnitY);
                canvas.printf("m/s");
                canvas.setTextSize(2);
                canvas.setCursor(UnitX+50, UnitY-10);
                canvas.printf("2");
            } //mode 0,2     

                
            break;
        case 1:     
            for(int i = 0; i < SENSOR::numberOfDevices; i++) {
                defaultDisplay = false;
                canvas.setCursor(10, 23+(i*23));
                canvas.setTextColor(TFT_CYAN);
                canvas.setTextSize(3);canvas.print("T");
                canvas.setTextSize(2);canvas.print(i+1);
                canvas.setTextSize(3);canvas.print(":");
                canvas.setTextSize(3);canvas.printf("%.1f", SENSOR::sensorReadings[i]);
            }
                // Print the sensor unit to the sprite canvas
                canvas.setTextColor(TFT_WHITE);
                static const int degreesX = 200;
                static const int degreesY = 100;
                canvas.setTextSize(2);
                canvas.setCursor(degreesX, degreesY-10);
                canvas.printf("o");
                canvas.setTextSize(3);
                canvas.setCursor(degreesX+15, degreesY);
                canvas.printf("C");
            break;
        
        default:
            break;
            
    }

    if(defaultDisplay){
        //print the sensor name to the sprite canvas
        canvas.setCursor(1,28);
        canvas.setTextColor(TFT_WHITE);
        canvas.setTextSize(2);
        String quantitySymbol = SENSOR::sensorDetails[SENSOR::sensorID[0]][SENSOR::sensorID[1]][1];
        int separatorIndex = quantitySymbol.indexOf('/');
        String quantity = (separatorIndex != -1) ? quantitySymbol.substring(0, separatorIndex) : quantitySymbol;
        canvas.print(quantity);

        //print the sensor unit to the sprite canvas
        canvas.setTextColor(TFT_WHITE);
        canvas.setTextSize(3);
        canvas.setCursor(15,95);
        canvas.printf("%s", SENSOR::sensorDetails[SENSOR::sensorID[0]][SENSOR::sensorID[1]][2]);

        //print the sensor value to the sprite canvas
        canvas.setTextSize(5);
        canvas.setCursor(15, 52);
        canvas.setTextColor(TFT_YELLOW);
        canvas.printf("%.2f", SENSOR::sensorReadings[0]*SENSOR::conversionFactor);

        //print the burst mode to the sprite canvas
        if(burstMode){
            canvas.setCursor(170, 95);
            canvas.setTextColor(TFT_GREEN);
            canvas.setTextSize(2);
            canvas.print("Burst:");
            canvas.setCursor(170, 113);
            canvas.print("PRESS");
        }
        else{
            canvas.setCursor(165, 95);
            canvas.setTextColor(TFT_WHITE);
            canvas.setTextSize(2);
            canvas.print("Normal");
        }

    }

        canvas.setTextColor(TFT_WHITE);

        
        // Print the battery percentage to the sprite canvas in green at the top right corner
        canvas.setTextSize(2);
        canvas.setCursor(158, 3);
        //set the text colour based on the battery percentage
        if(batteryPercentage < 20){
            canvas.setTextColor(TFT_RED);
        }
        else if(batteryPercentage < 40){
            canvas.setTextColor(TFT_ORANGE);
        }
        else{
            canvas.setTextColor(TFT_GREEN);
        }
        canvas.printf("%d%%", batteryPercentage);

        //print the connection status
        
        if(deviceConnected){
            canvas.setCursor(215,3);
            canvas.setTextSize(2);
            canvas.setTextColor(TFT_SKYBLUE);
            canvas.print("BT");
        }
        else{
            canvas.setCursor(220,2);
            canvas.setTextSize(2);
            canvas.setTextColor(TFT_RED);
            canvas.print("x");
        }
        canvas.setTextColor(TFT_WHITE);

        // Push the sprite to the screen
        canvas.pushSprite(0, 0);
    
}
}

void callibrationRoutine(){
    displayFree = false; // display cannot update as usual
    canvas.fillSprite(TFT_BLACK);
            canvas.setCursor(10, 23);
            canvas.setTextColor(TFT_YELLOW);
            canvas.setTextSize(3);
            canvas.print("Burst mode");
            canvas.setCursor(10, 46);
            canvas.print("HOLD STILL:");
            canvas.setCursor(10, 69);
            canvas.print("calibrating");
            canvas.setCursor(10, 92);
            canvas.setTextSize(6);
            canvas.print("3");
            if(loud){M5.Speaker.tone(5000, 200);}
            canvas.pushSprite(0, 0);
            delay(1000);
            
            canvas.fillSprite(TFT_BLACK);
            canvas.setCursor(10, 23);
            canvas.setTextColor(TFT_YELLOW);
            canvas.print("2");
            if(loud){M5.Speaker.tone(4000, 200);}
            canvas.pushSprite(0, 0);
            delay(1000);
           
            canvas.fillSprite(TFT_YELLOW);
            canvas.setCursor(10, 23);
            canvas.setTextColor(TFT_BLACK);
            canvas.print("1");
            if(loud){M5.Speaker.tone(3000, 200);}
            canvas.pushSprite(0, 0);
            delay(500);
            SENSOR::callibrateAcc(); //takes 25ms
            delay(175);
            canvas.fillSprite(TFT_GREEN);
            canvas.pushSprite(0, 0);
            delay(300);
            if(loud){M5.Speaker.tone(6000, 500);}
            displayFree = true; //unblocks the display update

            
}

void message(char msg, int size, int displayFor, bool persistAfter){
    displayFree = false; // display cannot update as usual
    canvas.fillSprite(TFT_BLACK);
    canvas.setCursor(10, 23);
    canvas.setTextColor(TFT_YELLOW);
    canvas.setTextSize(size);
    canvas.print(msg);
    canvas.pushSprite(0, 0);
    delay(displayFor);
    displayFree = true;
    if(persistAfter == 0){
        displayFree = true;
    }
}

void recordingMessage(){
    
    displayFree = false; // display cannot update as usual
    canvas.fillSprite(TFT_GREEN);
    canvas.setCursor(10, 23);
    canvas.setTextColor(TFT_BLACK);
    canvas.setTextSize(4);
    canvas.print("Recording");
    
    canvas.pushSprite(0, 0);
}

void TransmittingMessage(){
    
    displayFree = false; // display cannot update as usual
    canvas.fillSprite(TFT_CYAN);
    canvas.setCursor(10, 23);
    canvas.setTextColor(TFT_BLACK);
    canvas.setTextSize(4);
    canvas.print("Sending");
    
    canvas.pushSprite(0, 0);
}


//menu system
bool menuOpen = false;
int menuScreen = 0;
int menuSelection = 0;
int menuSelectionDisplayMax = 4; //show only 4 options
int menuSelectionDelta = 0; //shifts the menu down
String menuTree[4][8] = { //[menuScreen][menuSelection]
    {"Sensor", "Interval", "Beep", "Exit"},
    {"Microphone", "Accelerometer", "Net Acceleration", "Velocity", "Displacement", "Gyroscope", "Magnetometer", "Exit"},
    {"3", "5", "10", "20", "50", "100", "1000", "Exit"},
    {"On", "Off", "Exit"}
};
String intervalOptions[2][7] = {
    {"20", "500", "1000", "5000", "10000", "30000", "60000"}, //non burst modes (millis)
    {"3", "5", "10", "20", "25", "50", "100"}//burse modes (millis)
    
};

void buildMenuTree(){
    for(int i = 0; i < 7; i++){
        menuTree[1][i] = SENSOR::sensorDetails[SENSOR::sensorID[0]][i][0];
        if(String(menuTree[1][i]).length() > 11){
            menuTree[1][i] = String(menuTree[1][i]).substring(0, 11);
        }
        menuTree[2][i] = intervalOptions[burstMode][i];
    }
}

int menuSelectionLength[4] = {4, 8, 8, 3};


void menuDraw(){
    displayFree = false; // display cannot update as usual
    canvas.fillSprite(TFT_BLACK);
    canvas.setCursor(0, 0);
    canvas.setTextColor(TFT_YELLOW);
    canvas.setTextSize(3);
    if(menuScreen == 0){
        canvas.print("Menu");
    }
    else{
        canvas.print(menuTree[0][menuScreen-1]);
    }

    for (int i = 0; i < menuSelectionDisplayMax; i++){
        canvas.setCursor(15, 23+(i*23));
        if(i + menuSelectionDelta == menuSelection){
            canvas.setTextColor(TFT_GREEN);
        }
        else{
            canvas.setTextColor(TFT_WHITE);
        }
        canvas.print(menuTree[menuScreen][i + menuSelectionDelta]);//print the menu item and move along if the diplay has cycled
    }
    canvas.pushSprite(0,0);
    
    
}

void menuChangeDraw(String line1, String line2, String line3){
    displayFree = false; // display cannot update as usual
    canvas.fillSprite(TFT_WHITE);
    canvas.setCursor(15, 0);
    canvas.setTextColor(TFT_BLACK);
    canvas.setTextSize(3);
    if(menuScreen == 0){
        canvas.print("Menu");
    }
    else{
        canvas.print(menuTree[0][menuScreen-1]);
    }

    canvas.setTextColor(TFT_RED);
    for (int i = 0; i < 3; i++){
        canvas.setCursor(15, 33+(i*23));
        switch (i){
            case 0:
            if(String(line1).length() > 11){
                line1 = String(line1).substring(0, 11);
            }
                canvas.print(line1);
                break;
            case 1:
                canvas.print(line2);
                break;
            case 2:
                canvas.print(line3);
                break;
        }
    }
    canvas.pushSprite(0,0);
    delay(2000);
}
