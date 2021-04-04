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

  if(stage == 0){
    // Open title card welcome
    tft.setCursor(0, 0);
    tft.setTextColor(0xFFFF);
    tft.setTextWrap(true);
    tft.print("They're coming...");
  
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
    // @todo more welcome stages
  }

  
}
