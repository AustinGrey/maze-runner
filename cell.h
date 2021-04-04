#ifndef CELL_H
#define CELL_H
/*
 * Defines the cell class, which is a place in the maze that could be walked on
 */
#include "types.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735

class Cell{
  public:
    // Helper, determines if this cell is visited during maze gen
    bool visited = false;
    // Determines what direction the maze gen alg went when it last visited this cell, if unvisited, is none
    Direction lastWalked = NONE;
    // If this cell is the entrance to the maze
    bool isEntrance = false;
    // If this cell is the exit to the maze
    bool isExit = false;
    // Cells known position in the maze, (posX, posY)=(0,0) at top left
    int posX;
    int posY;
    // Draw to the given display with the given information
    void draw(
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
      );
    String toString();
};

#endif
