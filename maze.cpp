#include "maze.h"
#include "types.h"
#include "edge.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <LinkedList.h>

int Maze::COLOR_CELL_ENTRANCE;
int Maze::COLOR_CELL_EXIT;
int Maze::COLOR_CELL_VISITED;
int Maze::COLOR_CELL_UNVISITED;

int Maze::COLOR_EDGE_BLOCKING;
int Maze::COLOR_EDGE_OPEN;

Maze::Maze(Adafruit_ST7735* tft_ptr, int numCols, int numRows){
  this->numCols = numCols;
  this->numRows = numRows;

  start = cells[0];
  finish = cells[numCols * numRows - 1];

  this->tft_ptr = tft_ptr;
  auto& tft = *tft_ptr;

  // Initialize information about how cells' size and position within this maze given the display
  cellWidth = (tft.width() - 2) / numCols;
  cellHeight = (tft.height() - 2) / numRows;
  // Calculate the spacing that would go around this maze so that it is centered since we do not allow fractional cell widths and heights
  paddingY = ((tft.height() - 2) - (cellHeight * numRows)) / 2;
  paddingX = ((tft.width() - 2) - (cellWidth * numCols)) / 2;

  // Initialize colors for cells and edges in this maze, given the display
  COLOR_CELL_ENTRANCE = tft.color565(200, 50, 50);
  COLOR_CELL_EXIT = tft.color565(50, 200, 50);
  COLOR_CELL_VISITED = 0xFFFF;
  COLOR_CELL_UNVISITED = tft.color565(100, 100, 170);
  
  COLOR_EDGE_BLOCKING = tft.color565(0, 0, 0);
  COLOR_EDGE_OPEN = COLOR_CELL_VISITED;

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
}

/**
 * Draws the maze to it's current display
 */
void Maze::draw(){
  for(auto&& cell: cells){
    // Only draw cells that belong on the current size
    if(cell.posX >= numCols || cell.posY >= numRows) continue;
    cell.draw(
      *tft_ptr,
      cell.isEntrance
        ? COLOR_CELL_ENTRANCE
        : cell.isExit
          ? COLOR_CELL_EXIT
          : cell.visited 
            ? COLOR_CELL_VISITED 
            : COLOR_CELL_UNVISITED,
      cellWidth,
      cellHeight,
      paddingX,
      paddingY,
      getCellEdge(cell.posX, cell.posY, north).isBlocking,
      getCellEdge(cell.posX, cell.posY, east).isBlocking,
      getCellEdge(cell.posX, cell.posY, south).isBlocking,
      getCellEdge(cell.posX, cell.posY, west).isBlocking,
      COLOR_EDGE_BLOCKING,
      COLOR_EDGE_OPEN
      );
  }
}

/**
 * For the given cell, returns the edge of that cell in the given direction
 */
Edge& Maze::getCellEdge(int cellX, int cellY, Direction dir){
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

/**
 * Gets the cell at the given position
 */
Cell* Maze::getCell(int col, int row){
  return &cells[col + (row * numCols)];
}

/**
 * Gets a pointer to a random cell that is unvisited
 * If all cells have been visited, returns a null pointer
 */
Cell* Maze::getUnvisitedCell(){
  // Gather list of unvisited cells
  int cellArrSize = sizeof(cells[0]) * (numCols * numRows - 1);
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

void Maze::genMaze(){
  // Mark a random cell as part of the maze (the seed cell)
  getUnvisitedCell()->visited = true;

  
  while(genPath()){
    // genPaths until you cannot any more
  }
  
  // Mark the start and end
  getCell(0, 0)->isEntrance = true;
  drawCell(0, 0);
  getCell(numCols - 1, numRows - 1)->isExit = true;
  drawCell(numCols - 1, numRows - 1);
}

/**
 * Generates a new path in the maze. Redraws any cells that are newly visited
 * Returns true if successful, false if it was unable to (usually because there are no unvisited cells to generate a path for)
 * Requires: at least one seed cell has been marked visited
 */
bool Maze::genPath(){
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
    drawCell(currentCell->posX, currentCell->posY);
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
  drawCell(currentCell->posX, currentCell->posY);

  return true;
}

void Maze::drawCell(int cellX, int cellY){
  Cell& cell = *getCell(cellX, cellY);
  cell.draw(
    *tft_ptr,
    cell.isEntrance
      ? COLOR_CELL_ENTRANCE
      : cell.isExit
        ? COLOR_CELL_EXIT
        : cell.visited 
          ? COLOR_CELL_VISITED 
          : COLOR_CELL_UNVISITED,
    cellWidth,
    cellHeight,
    paddingX,
    paddingY,
    getCellEdge(cell.posX, cell.posY, north).isBlocking,
    getCellEdge(cell.posX, cell.posY, east).isBlocking,
    getCellEdge(cell.posX, cell.posY, south).isBlocking,
    getCellEdge(cell.posX, cell.posY, west).isBlocking,
    COLOR_EDGE_BLOCKING,
    COLOR_EDGE_OPEN
    );
}
