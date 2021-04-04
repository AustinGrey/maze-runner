/*
 * Defines the cell class, which is a place in the maze that could be walked on
 */
#include "types.h"

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
};
