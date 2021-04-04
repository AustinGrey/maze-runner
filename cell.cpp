#include "cell.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
/**
 * Draws this cell to the provided display, requires a reference to the maze this cell belongs to
 */
void Cell::draw(
  Adafruit_ST7735& tft, 
  int color, 
  int width, 
  int height, 
  int paddingX, 
  int paddingY, 
  bool northEdgeBlocking,
  bool eastEdgeBlocking,
  bool southEdgeBlocking,
  bool westEdgeBlocking,
  int blockedEdgeColor,
  int openEdgeColor
  ){
  tft.fillRect(
    (posX * width) + paddingX, 
    (posY * height) + paddingY, 
    width, 
    height, 
    color
    );
    // Draw the cell's position to verify correct output
//      tft.setTextColor(tft.color565(255, 0, 0));
//      tft.setCursor((cell.posX * cellWidth) + paddingX, (cell.posY * cellHeight) + paddingY);
//      tft.print(cell.posY);
    // Draw the top edge of this cell
  tft.drawFastHLine(
    (posX * width) + paddingX, 
    (posY * height) + paddingY, 
    width,
    northEdgeBlocking ? blockedEdgeColor : openEdgeColor
    );
  // Draw the left edge of this cell
  tft.drawFastVLine(
    (posX * width) + paddingX, 
    (posY * height) + paddingY, 
    height,
    westEdgeBlocking ? blockedEdgeColor : openEdgeColor
    );
//  // If this is a bottom cell, draw the bottom edge for the cell
//  if(posY == numRows - 1){
//    tft.drawFastHLine(
//    (posX * width) + paddingX, 
//    ((posY+1) * height) + paddingY, 
//    width,
//    southEdgeBlocking ? blockedEdgeColor : openEdgeColor
//    );
//  }
//  // If this is a right-most cell, draw the right edge for the cell
//  if(posX == numCols - 1){
//    tft.drawFastVLine(
//    ((posX+1) * width) + paddingX, 
//    (posY * height) + paddingY, 
//    height,
//    eastEdgeBlocking.isBlocking ? blockedEdgeColor : openEdgeColor
//    );
//  }
}

String Cell::toString(){
  return String("Cell(")
    +String(posX)
    +String(",")
    +String(posY)
    +String(")");
}
