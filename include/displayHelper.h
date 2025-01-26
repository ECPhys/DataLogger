#pragma once

//forward declaration of the battery percentage variable
    int batteryPercentage = 0;

// Create a sprite canvas
    M5Canvas canvas(&M5.Lcd);
    int DISPLAY_BRIGHTNESS = 2;

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
    //clear the canvas
    canvas.fillSprite(TFT_BLACK);
    // Set the font size for the timer and temperature
    canvas.setTextSize(3); // Increase the font size
    

    // Print the timer to the sprite canvas
    canvas.setCursor(0, 0);
    canvas.setTextColor(WHITE, BLACK);
    canvas.printf("%02lu:%02lu:%02lu", hours, minutes, seconds);
    
    // Print the sensor data to the sprite canvas
switch (SENSOR::sensorID[0]) {
    case 0:
        canvas.setCursor(10, 23);
        if(SENSOR::sensorID[1] == 1){
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
        
            canvas.setTextColor(TFT_CYAN);
            canvas.setCursor(10, 23);
            
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
        
        canvas.setTextColor(TFT_YELLOW);
        canvas.setTextSize(3);canvas.print("a:");
        if(SENSOR::sensorID[1] == 1) 
        {canvas.setTextSize(3);canvas.printf("%.2f", SENSOR::sensorReadings[4]*SENSOR::conversionFactor);} //mode 0,1
        else if(SENSOR::sensorID[1] == 2){
            canvas.setTextSize(3);canvas.printf("%.2f", SENSOR::sensorReadings[0]*SENSOR::conversionFactor);
        } //mode 0,2
        

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
            
        break;
    case 1:     
        for(int i = 0; i < SENSOR::numberOfDevices; i++) {
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

    canvas.setTextColor(TFT_WHITE);

    
    // Print the battery percentage to the sprite canvas in green at the top right corner
    canvas.setTextSize(3);
    canvas.setCursor(160, 0);
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

    // Push the sprite to the screen
    canvas.pushSprite(0, 0);
    
}

void callibrationRoutine(){
    canvas.fillSprite(TFT_BLACK);
            canvas.setCursor(10, 23);
            canvas.setTextColor(TFT_YELLOW);
            canvas.setTextSize(3);
            canvas.print("Burst mode");
            canvas.setCursor(10, 46);
            canvas.print("HOLD STILL: calibrating");
            canvas.setCursor(10, 69);
            canvas.setTextSize(4);
            canvas.print("3");
            M5.Speaker.tone(5000, 300);
            canvas.pushSprite(0, 0);
            delay(1000);
            canvas.fillSprite(TFT_BLACK);
            canvas.setCursor(10, 23);
            canvas.setTextColor(TFT_YELLOW);
            canvas.print("2");
            M5.Speaker.tone(4000, 300);
            canvas.pushSprite(0, 0);
            delay(1000);
            canvas.fillSprite(TFT_BLACK);
            canvas.setCursor(10, 23);
            canvas.setTextColor(TFT_YELLOW);
            canvas.print("1");
            M5.Speaker.tone(3000, 300);
            canvas.pushSprite(0, 0);
            delay(500);
}