#include <LinkedList.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include "edge.h"
#include "cell.h"
#include "types.h"
#include "welcome.h"

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
const int numRows = 8;
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

// Tracks every cell in the 2d grid. To get the [i][j]th cell, use [i * numCols + j]
Cell cells[numCols*numRows];

// Stores edges, first all north/south edges, then all east/west edges in row major order
// (w * (h + 1)) + (h * (w + 1))
Edge edges[(numCols * (numRows + 1)) + (numRows * (numCols + 1))];

// The class used to define the player object for game state tracking
class Player{
  public:
    int row = 0;
    int col = 0;
};
Player player;

void setup(void) {
  Serial.begin(9600);
  Serial.print("Start program");

  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab

  // OR use this initializer if using a 1.8" TFT screen with offset such as WaveShare:
  // tft.initR(INITR_GREENTAB);      // Init ST7735S chip, green tab
  
  // SPI speed defaults to SPI_DEFAULT_FREQ defined in the library, you can override it here
  // Note that speed allowable depends on chip and quality of wiring, if you go too fast, you
  // may end up with a black screen some times, or all the time.
  //tft.setSPISpeed(40000000);

  // Setup the input pin for the joystick sel
  pinMode(PIN_JOYSTICK_SEL, INPUT_PULLUP);

  Serial.println("Init done");

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  Serial.println(time, DEC);
  delay(500);

  displayWelcomeForStage(tft, 0);

  // Init the cells
  for(int i = 0; i < (sizeof(cells) / sizeof(cells[0])); i++){
    cells[i].posX = i % numCols;
    cells[i].posY = i / numCols;
//    Serial.print("Cell[");
//    Serial.print(i);
//    Serial.print("]: (");
//    Serial.print(cells[i].posX);
//    Serial.print(", ");
//    Serial.print(cells[i].posY);
//    Serial.println(")");
  }

  // Init the edges
//  for(int i = 0; i < (sizeof(edges) / sizeof(edges[0])); i++){
//    edges[i].isBlocking = false;
//  }

  // Init the player
//  player = new Player();

  // Reset screen
  tft.fillScreen(0x0000);
  
  genMaze();
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
  return getCell(player.col, player.row)->isExit;
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

Cell* getCell(int x, int y){
  return &cells[x + (y * numCols)];
}

/**
 * Gets a pointer to a random cell that is unvisited
 * If all cells have been visited, returns a null pointer
 */
Cell* getUnvisitedCell(){
  // Gather list of unvisited cells
  int cellArrSize = sizeof(cells) / sizeof(cells[0]);
  int numUnvisited = 0;
  LinkedList<Cell*> unvisitedCells;
  for(int i = 0; i < cellArrSize; i++){
    if(!cells[i].visited){
      unvisitedCells.add(&cells[i]);
      numUnvisited++;
    }
  }

  if(numUnvisited == 0){
    return NULL;
  } else {
    return unvisitedCells.get(random(unvisitedCells.size()));
  }
}

/**
 * For the given cell, returns the edge of that cell in the given direction
 */
Edge& getCellEdge(int cellX, int cellY, Direction dir){
  int eastWestOffset = numCols * (numRows + 1);
  switch(dir)
  {
    // N/S edges are stored first, so no need to jump to middle of array
    case north: return edges[cellX + (cellY * numCols)];  
    case south: return edges[cellX + ((cellY + 1) * numCols)];
    // E/W edges stored last, must skip all N/S edges first
    case west: return edges[eastWestOffset + cellX + (cellY * (numCols + 1))];
    case east: return edges[eastWestOffset + (cellX + 1) + (cellY * (numCols + 1))];
  }
}

void genMaze(){
  // Mark a random cell as part of the maze (the seed cell)
  getUnvisitedCell()->visited = true;

  
  while(genPath()){
    // genPaths until you cannot any more
  }
  
  // Mark the start and end
  getCell(0, 0)->isEntrance = true;
  drawCell(*getCell(0, 0));
  getCell(numCols - 1, numRows - 1)->isExit = true;
  drawCell(*getCell(numCols - 1, numRows - 1));
}

/**
 * Generates a new path in the maze. Redraws any cells that are newly visited
 * Returns true if successful, false if it was unable to (usually because there are no unvisited cells to generate a path for)
 * Requires: at least one seed cell has been marked visited
 */
bool genPath(){
  // Get a new unvisited cell and create a loop reduced path to any visited cell
  Cell* pathStart = getUnvisitedCell();
  if(pathStart == NULL) return false;
  
  Cell* currentCell = pathStart;
  Direction dirBackToLastCell = NONE;
  while(!currentCell->visited){
    // Find a valid direction we can walk from this cell
    bool dirIsValid = false;
    Direction dir;
    while(!dirIsValid){
      dir = Direction(random(FINAL_VALID + 1));
      if(
        (dir == north && currentCell->posY == 0)
        || (dir == east && currentCell->posX == numCols - 1)
        || (dir == south && currentCell->posY == numRows - 1)
        || (dir == west && currentCell->posX == 0)
        || (dir == dirBackToLastCell)
        ){
          dirIsValid = false;
//          Serial.print("Rejected option ");
//          Serial.print(dir);
//          Serial.print(" on cell (");
//          Serial.print(currentCell->posX);
//          Serial.print(", ");
//          Serial.print(currentCell->posY);
//          Serial.println(")");
        }
        else {
          dirIsValid = true;
        }
    }
//    Serial.print("Walking ");
//    Serial.print(dir == north ? "north" : dir == east ? "east" : dir == south ? "south" : dir == west ? "west" : "err");
//    Serial.print(" on cell (");
//    Serial.print(currentCell->posX);
//    Serial.print(", ");
//    Serial.print(currentCell->posY);
//    Serial.println(")");
    // Mark that as the direction we walked for this cell
    currentCell->lastWalked = dir;
    // 
    // Get the neighbor in the chosen direction and continue, and prevent us from immediately backtracking
    switch(dir){
      case north: 
        currentCell = getCell(currentCell->posX, currentCell->posY - 1);
        dirBackToLastCell = south;
        break;
      case east: 
        currentCell = getCell(currentCell->posX + 1, currentCell->posY);
        dirBackToLastCell = west;
        break;
      case south: 
        currentCell = getCell(currentCell->posX, currentCell->posY + 1);
        dirBackToLastCell = north;
        break;
      case west: 
        currentCell = getCell(currentCell->posX - 1, currentCell->posY);
        dirBackToLastCell = east;
        break;
    }
  }
  Serial.println("PATH DISCOVERED");
  // Walk the discovered path and setup the edges for those cells
  currentCell = pathStart;
  while(!currentCell->visited){
//    Serial.print("Walking ");
//    Serial.print(currentCell->lastWalked);
//    Serial.print(currentCell->lastWalked == north ? "north" : currentCell->lastWalked == east ? "east" : currentCell->lastWalked == south ? "south" : currentCell->lastWalked == west ? "west" : "err");
//    Serial.print(" on cell (");
//    Serial.print(currentCell->posX);
//    Serial.print(", ");
//    Serial.print(currentCell->posY);
//    Serial.println(")");
    currentCell->visited = true;
    getCellEdge(currentCell->posX, currentCell->posY, currentCell->lastWalked).isBlocking = false;
    drawCell(*currentCell);
    switch(currentCell->lastWalked){
      case north:
        currentCell = getCell(currentCell->posX, currentCell->posY - 1);
        break;
      case east: 
        currentCell = getCell(currentCell->posX + 1, currentCell->posY);
        break;
      case south: 
        currentCell = getCell(currentCell->posX, currentCell->posY + 1);
        break;
      case west: 
        currentCell = getCell(currentCell->posX - 1, currentCell->posY);
        break;
    }
  }
  drawCell(*currentCell);

  return true;
}

void drawMaze(){
  for(auto&& cell: cells){
    drawCell(cell);
  }
}

void drawCell(Cell& cell){
  tft.fillRect(
    (cell.posX * cellWidth) + paddingX, 
    (cell.posY * cellHeight) + paddingY, 
    cellWidth, 
    cellHeight, 
    cell.isEntrance
      ? COLOR_CELL_ENTRANCE
      : cell.isExit
        ? COLOR_CELL_EXIT
        : cell.visited 
          ? COLOR_CELL_VISITED 
          : COLOR_CELL_UNVISITED
    );
    // Draw the cell's position to verify correct output
//      tft.setTextColor(tft.color565(255, 0, 0));
//      tft.setCursor((cell.posX * cellWidth) + paddingX, (cell.posY * cellHeight) + paddingY);
//      tft.print(cell.posY);
    // Draw the top edge of this cell
  tft.drawFastHLine(
    (cell.posX * cellWidth) + paddingX, 
    (cell.posY * cellHeight) + paddingY, 
    cellWidth,
    getCellEdge(cell.posX, cell.posY, north).isBlocking ? COLOR_EDGE_BLOCKING : COLOR_EDGE_OPEN
    );
  // Draw the left edge of this cell
  tft.drawFastVLine(
    (cell.posX * cellWidth) + paddingX, 
    (cell.posY * cellHeight) + paddingY, 
    cellHeight,
    getCellEdge(cell.posX, cell.posY, west).isBlocking ? COLOR_EDGE_BLOCKING : COLOR_EDGE_OPEN
    );
  // If this is a bottom cell, draw the bottom edge for the cell
  if(cell.posY == numRows - 1){
    tft.drawFastHLine(
    (cell.posX * cellWidth) + paddingX, 
    ((cell.posY+1) * cellHeight) + paddingY, 
    cellWidth,
    getCellEdge(cell.posX, cell.posY, south).isBlocking ? COLOR_EDGE_BLOCKING : COLOR_EDGE_OPEN
    );
  }
  // If this is a right-most cell, draw the right edge for the cell
  if(cell.posX == numCols - 1){
    tft.drawFastVLine(
    ((cell.posX+1) * cellWidth) + paddingX, 
    (cell.posY * cellHeight) + paddingY, 
    cellHeight,
    getCellEdge(cell.posX, cell.posY, east).isBlocking ? COLOR_EDGE_BLOCKING : COLOR_EDGE_OPEN
    );
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
  Cell& currentCell = *getCell(player.col, player.row);
  if(getCellEdge(currentCell.posX, currentCell.posY, dir).isBlocking){
    // The move is illegal and nothing happens
    Serial.print("Illegal move through wall:");
    Serial.println(dir);
    return;
  }
  Serial.print("Moving: ");
  Serial.println(dir);
  // Redraw the cell that is leaving
  drawCell(currentCell);

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
