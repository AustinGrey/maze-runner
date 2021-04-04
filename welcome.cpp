#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
/**
 * Welcomes the user depending on what stage they are on. The 0th stage welcome is the one intended to be shown before the 1st maze is run
 * Takes the screen to output the welcome to, and which welcome to display
 */
void displayWelcomeForStage(Adafruit_ST7735 tft, int stage){
  const int textHeight = 6; //px
  const int textWidth = 5; //px
  
  // Reset screen
  tft.fillScreen(0x0000);

  // Set text cursor up
  tft.setCursor(0, 0);
  tft.setTextColor(0xFFFF);
  tft.setTextWrap(true);

  if(stage == 0){
    // Open title card welcome
    tft.print("'They're coming...'");
  
    const char* splashText = "RUUNNNNN";
    int yOffset = 12;
    int cursorX = (tft.width()- ((textWidth+1)*strlen(splashText))) / 2;
    int cursorY = ((tft.height()- textHeight) / 2) - yOffset;
    // All the way down
    float colorInterval = 255/yOffset;
    for(int i = 0; i <= yOffset; i++){
      cursorY++;
      tft.setTextColor(tft.color565(i * colorInterval, i * colorInterval / (i*i), i * colorInterval / (i*i)));
      tft.setCursor(cursorX, cursorY);
      tft.print(splashText);
      delay(100);
    }
  }
  else if (stage == 1){
    // Open title card welcome
    tft.print("The door is locked,\nbut old.\n\n"); delay(3000);
    tft.print("With both of your\nstrength you kick it\ndown and keep running"); delay(3000);
  } 
  else if (stage == 2){
    tft.print("The barbed wire is\nmissing on this wall,\nyou could climb over!\n\n"); delay(3000);
    tft.print("They help you up\nbut the top is slick\nwith algae\n\n");delay(3000);
    tft.print("You fall onto the\nother side before you\ncan help them over."); delay(3000);
  }
  else if (stage == 3){
    tft.print("You hear screaming in\nthe distance."); delay(3000);
  }
  else if (stage == 4){
    tft.print("You know you're\ngetting farther, but\nwhat are you getting\ncloser to?"); delay(3000);
  }
  else if (stage == 5){
    tft.print("You slam the door\nshut. Behind it you\nhear footsteps\napproaching"); delay(3000);
  }

  
}
