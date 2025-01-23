#pragma once

// Create a sprite canvas
    M5Canvas canvas(&M5.Lcd);

void DisplayInit(){
    // Set the brightness of the display
    M5.Display.setBrightness(2);
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
    
    // Print the temperature label to the sprite canvas
        
    for(    int i = 0; i < SENSOR::numberOfDevices; i++) {
        canvas.setCursor(10, 23+(i*23));
        canvas.setTextColor(TFT_CYAN);
        canvas.setTextSize(3);canvas.print("T");
        canvas.setTextSize(2);canvas.print(i+1);
        canvas.setTextSize(3);canvas.print(":");
        canvas.setTextSize(3);canvas.printf("%.1f", SENSOR::sensorReadings[i]);
    }

    canvas.setTextColor(TFT_WHITE);

    // Print the sensor unit to the sprite canvas
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
    
}
