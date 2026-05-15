// ============================================================
// Environment.h
// Obstacle & Environment System
// Manages the 2D grid, obstacles, and object placement.
// All other modules interact with the world through this class.
// ============================================================

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <iostream>
#include <vector>
#include <cstdlib>   // rand(), srand()
#include <ctime>     // time() for seed
#include <stdexcept> // runtime_error
#include "SimInterfaces.h" // adds DroneSim::IEnvironment, DroneSim::Point

// ---------------------------------------------------------
// Grid dimensions — change here to resize the entire world
// ---------------------------------------------------------
const int ROWS = 10;
const int COLS = 10;

// ---------------------------------------------------------
// CellType: represents what occupies each grid cell
// Using an enum class for type safety (no accidental int mix-up)
// ---------------------------------------------------------
enum class CellType {
    EMPTY    = 0,  // free space
    OBSTACLE = 1,  // impassable wall
    DRONE    = 2,  // drone's current position
    TARGET   = 3,  // navigation destination
    FUEL     = 4   // refueling station
};

// ---------------------------------------------------------
// Position: simple struct for (row, col) coordinates.
// Used across all modules for clean communication.
// ---------------------------------------------------------
struct Position {
    int row;
    int col;

    // Equality check — used in pathfinding comparisons
    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }
};

// ---------------------------------------------------------
// Environment: the single source of truth for world state.
// All modules query this class instead of managing their own maps.
// ---------------------------------------------------------
class Environment: public DroneSim::IEnvironment {
private:
    // The core grid — a 2D array of CellType values
    CellType grid[ROWS][COLS];

    // Track how many obstacles were placed
    int obstacleCount;

    // Internal helper: check if coordinates are within bounds
    bool isInBounds(int row, int col) const;

public:
    // --- Constructor ---
    Environment();

    // --- Core Setup ---
    void initializeGrid();
    void generateObstacles(int densityPercent = 25);

    // --- Query Functions (called by Drone & Pathfinder) ---
    bool isObstacle(int row, int col) const;
    bool isValidMove(int row, int col) const;
    CellType getCellType(int row, int col) const;

    // --- Object Placement ---
    bool placeObject(int row, int col, CellType type);
    bool clearObject(int row, int col);

    // --- Safe Spawning ---
    Position safeSpawn();

    // --- Manual Obstacle Placement ---
    void addObstacle(int row, int col);
    void removeObstacle(int row, int col);

    // --- Dynamic Obstacles ---
    void moveObstacle(Position from, Position to);

    // --- Statistics ---
    int getObstacleCount() const;

    // --- Debug: Console display ---
    void displayGrid() const;

    // --- Getters for grid dimensions (for SFML rendering) ---
    int getRows() const { return ROWS; }
    int getCols() const { return COLS; }


    // --- IEnvironment interface ---
    // Coordinate mapping: x → col, y → row
    // Allows Pathfinder and AILogicController to use this
    // class through the IEnvironment* interface pointer.
    bool isWalkable(int x, int y) const override;
    bool isValid(int x, int y)    const override;
    std::vector<DroneSim::Point> getFuelStations() const override;
};

#endif // ENVIRONMENT_H