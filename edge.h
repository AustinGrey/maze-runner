// There is an edge between every cell and between cells and the edges of the map
// @todo only define edges between cells, as the edges of the map are always implicit blocking edges
class Edge{
  public:
    bool isBlocking = true; // If the edge should stop movement (is a wall)
};
