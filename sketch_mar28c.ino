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

// Given the size of the maze, determine the size of the cells on this display
const int numRows = 10;
const int numCols = 8;

// The screen dimensions are always -2, to ensure at least a 1 px border around the maze for the outer edges to show
// We do not allow fractional cell widths and heights since that causes odd drawing behaviour
const int cellWidth = (tft.width() - 2) / numCols;
const int cellHeight = (tft.height() - 2) / numRows;
// Calculate the spacing that would go around this maze so that it is centered since we do not allow fractional cell widths and heights
const int paddingY = ((tft.height() - 2) - (cellHeight * numRows)) / 2;
const int paddingX = ((tft.width() - 2) - (cellWidth * numCols)) / 2;

const int COLOR_CELL_ENTRANCE = tft.color565(200, 50, 50);
const int COLOR_CELL_EXIT = tft.color565(50, 200, 50);
const int COLOR_CELL_VISITED = 0xFFFF;
const int COLOR_CELL_UNVISITED = tft.color565(100, 100, 170);

const int COLOR_EDGE_BLOCKING = tft.color565(0, 0, 0);
const int COLOR_EDGE_OPEN = COLOR_CELL_VISITED;

const int COLOR_PLAYER = tft.color565(0, 0, 255);


// Joystick
#define PIN_JOYSTICK_V 0 // Analog
#define PIN_JOYSTICK_H 1 // Analog
#define PIN_JOYSTICK_SEL 7 // Digital
#define JOYSTICK_DEADZONE_V 100
#define JOYSTICK_DEADZONE_H 100
#define JOYSTICK_V_RANGE 1023 // The possible range of values for the joystick, half of this is the center of the joystick
#define JOYSTICK_H_RANGE 1023 // The possible range of values for the joystick, half of this is the center of the joystick





Player player;

int currentLevel = 0;

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

  displayWelcomeForStage(tft, 0);

  // Create the maze
  maze = new Maze(&tft, currentLevel + 4, currentLevel + 4);

  // Reset screen
  tft.fillScreen(0x0000);
  
  maze->draw();

  drawPlayer();

  Serial.println("done");
}
bool hasWon = false;
void loop() {
  if(!hasWon){
    detectInput();
    hasWon = detectWin();
  }
  
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
    (player.col * cellWidth) + paddingX + (cellWidth / 2), 
    (player.row * cellHeight) + paddingY + (cellHeight / 2), 
    min(cellWidth, cellHeight) / 2 - 2, 
    COLOR_PLAYER);
}

void movePlayer(Direction dir){
  // Prevent illegal edge moves
  if(
    (dir == north && player.row == 0)
    || (dir == east && player.col == numCols - 1)
    || (dir == south && player.row == numRows - 1)
    || (dir == west && player.col == 0)
    )
    {
      // The move is illegal and nothing happens
      Serial.print("Illegal move through maze edge:");
      Serial.println(dir);
      return;
    }
  // Prevent illegal moves through walls
  Cell& currentCell = *maze->getCell(player.col, player.row);
  if(maze->getCellEdge(currentCell.posX, currentCell.posY, dir).isBlocking){
    // The move is illegal and nothing happens
    Serial.print("Illegal move through wall:");
    Serial.println(dir);
    return;
  }
  Serial.print("Moving: ");
  Serial.println(dir);
  // Redraw the cell that is leaving
  maze->drawCell(currentCell.posX, currentCell.posY);

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
