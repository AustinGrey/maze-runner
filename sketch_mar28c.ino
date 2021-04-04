#include <LinkedList.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include "edge.h"
#include "cell.h"
#include "types.h"
#include "welcome.h"
#include "player.h"
#include "maze.h"

#if defined(ARDUINO_FEATHER_ESP32) // Feather Huzzah32
  #define TFT_CS         14
  #define TFT_RST        15
  #define TFT_DC         32

#elif defined(ESP8266)
  #define TFT_CS         4
  #define TFT_RST        16                                            
  #define TFT_DC         5

#else
  // For the breakout board, you can use any 2 or 3 pins.
  // These pins will also work for the 1.8" TFT shield.
  #define TFT_CS        10
  #define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
  #define TFT_DC         8
#endif

// OPTION 1 (recommended) is to use the HARDWARE SPI pins, which are unique
// to each board and not reassignable. For Arduino Uno: MOSI = pin 11 and
// SCLK = pin 13. This is the fastest mode of operation and is required if
// using the breakout board's microSD card.

// For 1.44" and 1.8" TFT with ST7735 use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Joystick
#define PIN_JOYSTICK_V 0 // Analog
#define PIN_JOYSTICK_H 1 // Analog
#define PIN_JOYSTICK_SEL 7 // Digital
#define JOYSTICK_DEADZONE_V 100
#define JOYSTICK_DEADZONE_H 100
#define JOYSTICK_V_RANGE 1023 // The possible range of values for the joystick, half of this is the center of the joystick
#define JOYSTICK_H_RANGE 1023 // The possible range of values for the joystick, half of this is the center of the joystick


const int COLOR_PLAYER = tft.color565(0, 0, 255);


Player player;

// The current level we are on
int currentLevel = -1;
// If the player has won the current level
bool hasWon = true;

Maze* maze;

void setup(void) {
  Serial.begin(9600);
  Serial.print("Start program");

  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab

  // Setup the input pin for the joystick sel
  pinMode(PIN_JOYSTICK_SEL, INPUT_PULLUP);

  Serial.println("Init done");

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  Serial.println(time, DEC);
}
void loop() {
  if(!hasWon){
    detectInput();
    hasWon = detectWin();
  } else {
    // Bring player to next level and welcome them
    currentLevel++;
    displayWelcomeForStage(tft, currentLevel);

    // Reset state
    hasWon = false;
    player.row = 0;
    player.col = 0;
    
    // Create the maze
    maze = new Maze(&tft, currentLevel + 4, currentLevel + 4);
  
    // Reset screen
    tft.fillScreen(0x0000);
    
    maze->draw();
  
    drawPlayer();
  }
  
}
void startLevel(int level){
  
}

bool detectWin(){
  return maze->getCell(player.col, player.row)->isExit;
}

void detectInput(){
  // read all values from the joystick

  int vertical = analogRead(PIN_JOYSTICK_V); // will be 0-JOYSTICK_V_RANGE
  int horizontal = analogRead(PIN_JOYSTICK_H); // will be 0-JOYSTICK_H_RANGE
  int select = digitalRead(PIN_JOYSTICK_SEL); // will be HIGH (1) if not pressed, and LOW (0) if pressed

  // print out the values

  Serial.print("vertical: ");
  Serial.print(vertical, DEC);
  Serial.print(" horizontal: ");
  Serial.print(horizontal, DEC);
  Serial.print(" select: ");
  if (select == HIGH) {
    Serial.println("not pressed");
  }
  else {
    Serial.println("PRESSED!");
  }

  // Handle N/S movement
  if(vertical >= (JOYSTICK_V_RANGE / 2) + JOYSTICK_DEADZONE_V){
    movePlayer(south);
  } else if (vertical <= (JOYSTICK_V_RANGE / 2) - JOYSTICK_DEADZONE_V){
    movePlayer(north);
  }

  // Handle E/W movement
  if(horizontal >= (JOYSTICK_H_RANGE / 2) + JOYSTICK_DEADZONE_H){
    movePlayer(east);
  } else if (horizontal <= (JOYSTICK_H_RANGE / 2) - JOYSTICK_DEADZONE_H){
    movePlayer(west);
  }
}




void drawPlayer(){
  tft.fillCircle(
    (player.col * maze->cellWidth) + maze->paddingX + (maze->cellWidth / 2), 
    (player.row * maze->cellHeight) + maze->paddingY + (maze->cellHeight / 2), 
    min(maze->cellWidth, maze->cellHeight) / 2 - 2, 
    COLOR_PLAYER);
}

void movePlayer(Direction dir){
  // Do not allow illegal moves
  if(!maze->checkMove(player.col, player.row, dir)) return;

  // Move allowed, proceed
  Serial.print("Moving: ");
  Serial.println(dir);
  // Redraw the cell that is leaving
  maze->drawCell(player.col, player.row);

  // Update player position
  switch(dir){
    case north:
      player.row--;
      break;
    case east:
      player.col++;
      break;
    case south:
      player.row++;
      break;
    case west:
      player.col--;
      break;
  }

  Serial.print("New Location: ");
  Serial.print(player.col);
  Serial.print(", ");
  Serial.println(player.row);
  
  drawPlayer();
}
