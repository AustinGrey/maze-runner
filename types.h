enum Direction{
  north = 0,
  east = 1,
  south = 2,
  west = 3,
  FINAL_VALID = west, // The maximum enum value that indicates a valid direction, for use with random
  NONE, // Indicates no direction
};

// There is an edge between every cell and between cells and the edges of the map
class Edge{
  public:
    bool isBlocking = true; // If the edge should stop movement (is a wall)
};

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

class Player{
  public:
    int row = 0;
    int col = 0;
};
