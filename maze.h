#ifndef MAZE_H
#define MAZE_H

#include "cell.h"
#include "types.h"
#include "edge.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735

/*
 * A maze object holds all the cells and edges, and is responsible for determining their relationships to eachother
 */
class Maze{
  private:
    static const int MAX_COLS = 15;
    static const int MAX_ROWS = 15;

    // Tracks every cell in the 2d grid in row major order. To get the [i][j]th cell, use [i * numCols + j]
    Cell cells[MAX_COLS * MAX_ROWS]; // Arduino doesn't allow dynamic memory allocation easily, so we take the memory needed for the final level

    // Stores edges, first all north/south edges, then all east/west edges in row major order. Includes outside edges
    // Arduino doesn't allow dynamic memory allocation easily, so we take the memory needed for the final level
    // (w * (h + 1)) + (h * (w + 1))
    // Edge edges[(numCols * (numRows + 1)) + (numRows * (numCols + 1))];
    Edge edges[(MAX_COLS * (MAX_ROWS + 1)) + (MAX_ROWS * (MAX_COLS + 1))];
    
    

    // The cells the maze start and finish at
    Cell& start = cells[0];
    Cell& finish = cells[0];
    
    int numCols; // The current number of columns in the maze
    int numRows; // The current number of rows in the maze

    // The display this maze should draw to
    Adafruit_ST7735* tft_ptr;

    void genMaze();
    Cell* getUnvisitedCell();

    static int COLOR_CELL_ENTRANCE;
    static int COLOR_CELL_EXIT;
    static int COLOR_CELL_VISITED;
    static int COLOR_CELL_UNVISITED;
    
    static int COLOR_EDGE_BLOCKING;
    static int COLOR_EDGE_OPEN;

    bool genPath();
    int numCells();
    
  public:
    Maze(Adafruit_ST7735* tft_ptr, int numMazeCols, int numMazeRows);
    void draw();
    void drawCell(int cellX, int cellY);
    Edge& getCellEdge(int cellX, int cellY, Direction dir);
    Cell* getCell(int col, int row);

    // The shape of cells within the maze on the display this maze is for
    int cellWidth;
    int cellHeight;
    // The spacing around the maze to ensure it is centered since we don't allow fractional cell dimensions to fill the screen
    int paddingY;
    int paddingX;

    bool checkMove(int cellX, int cellY, Direction dir);
 };

#endif
