/**
 * Holds common types used by multiple parts of the program
 */
#ifndef TYPES_H
#define TYPES_H

enum Direction{
  north = 0,
  east = 1,
  south = 2,
  west = 3,
  FINAL_VALID = west, // The maximum enum value that indicates a valid direction, for use with random
  NONE, // Indicates no direction
};

#endif
