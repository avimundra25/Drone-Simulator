// ============================================================
// Environment.cpp
// Implementation of the Obstacle & Environment System
// ============================================================

#include "Environment.h"

// ------------------------------------------------------------
// Constructor: seed random number generator, initialize grid
// ------------------------------------------------------------
Environment::Environment() : obstacleCount(0) {
    srand(static_cast<unsigned int>(time(nullptr))); // seed once at startup
    initializeGrid();
}

// ------------------------------------------------------------
// initializeGrid()
// Resets every cell in the grid to EMPTY.
// Call this before generating a new map or restarting the sim.
// ------------------------------------------------------------
void Environment::initializeGrid() {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            grid[r][c] = CellType::EMPTY;
        }
    }
    obstacleCount = 0;
    std::cout << "[Environment] Grid initialized (" 
              << ROWS << "x" << COLS << ")\n";
}

// ------------------------------------------------------------
// isInBounds()
// Private guard: checks if (row, col) is inside the grid.
// Called before every grid access to prevent array overflows.
// ------------------------------------------------------------
bool Environment::isInBounds(int row, int col) const {
    return (row >= 0 && row < ROWS && col >= 0 && col < COLS);
}

// ------------------------------------------------------------
// generateObstacles()
// Randomly fills cells with obstacles based on density %.
// Density of 25 means ~25% of EMPTY cells become obstacles.
// Skips cells already occupied (drone, target, fuel).
// ------------------------------------------------------------
void Environment::generateObstacles(int densityPercent) {
    // Clamp density to a safe range
    if (densityPercent < 0)   densityPercent = 0;
    if (densityPercent > 60)  densityPercent = 60; // >60% makes maps unsolvable

    obstacleCount = 0;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            // Only place obstacles on EMPTY cells
            if (grid[r][c] == CellType::EMPTY) {
                int roll = rand() % 100; // random 0–99
                if (roll < densityPercent) {
                    grid[r][c] = CellType::OBSTACLE;
                    obstacleCount++;
                }
            }
        }
    }

    std::cout << "[Environment] Obstacles generated: " 
              << obstacleCount << " cells blocked\n";
}

// ------------------------------------------------------------
// isObstacle()
// Returns true if the cell at (row, col) is an obstacle.
// Called by pathfinding to determine traversal cost.
// ------------------------------------------------------------
bool Environment::isObstacle(int row, int col) const {
    if (!isInBounds(row, col)) return true; // treat out-of-bounds as obstacle
    return grid[row][col] == CellType::OBSTACLE;
}

// ------------------------------------------------------------
// isValidMove()
// ★ Core safety gate — called by Drone and Pathfinder.
// Returns true only if the cell is in-bounds AND not an obstacle.
// The drone must NEVER move without passing this check first.
// ------------------------------------------------------------
bool Environment::isValidMove(int row, int col) const {
    if (!isInBounds(row, col)) {
        return false; // out of bounds
    }
    if (grid[row][col] == CellType::OBSTACLE) {
        return false; // blocked by obstacle
    }
    return true; // safe to enter
}

// ------------------------------------------------------------
// getCellType()
// Returns the raw CellType of a cell.
// Used by SFML renderer to decide which color/sprite to draw.
// ------------------------------------------------------------
CellType Environment::getCellType(int row, int col) const {
    if (!isInBounds(row, col)) {
        throw std::out_of_range("[Environment] getCellType: coordinates out of range");
    }
    return grid[row][col];
}

// ------------------------------------------------------------
// placeObject()
// Places any CellType at a given position.
// Returns false if position is invalid or already occupied.
// ------------------------------------------------------------
bool Environment::placeObject(int row, int col, CellType type) {
    if (!isInBounds(row, col)) {
        std::cerr << "[Environment] placeObject: out of bounds (" 
                  << row << "," << col << ")\n";
        return false;
    }
    if (grid[row][col] == CellType::OBSTACLE && type != CellType::OBSTACLE) {
        std::cerr << "[Environment] placeObject: cell (" 
                  << row << "," << col << ") is blocked by obstacle\n";
        return false;
    }
    grid[row][col] = type;
    return true;
}

// ------------------------------------------------------------
// clearObject()
// Resets a specific cell back to EMPTY.
// Called when drone moves away from its current cell.
// ------------------------------------------------------------
bool Environment::clearObject(int row, int col) {
    if (!isInBounds(row, col)) {
        std::cerr << "[Environment] clearObject: out of bounds\n";
        return false;
    }
    // Don't accidentally clear an obstacle
    if (grid[row][col] == CellType::OBSTACLE) {
        std::cerr << "[Environment] clearObject: cannot clear an obstacle\n";
        return false;
    }
    grid[row][col] = CellType::EMPTY;
    return true;
}

// ------------------------------------------------------------
// safeSpawn()
// Finds a random EMPTY cell to safely place drone/target/fuel.
// Guarantees no spawn on top of obstacles or other objects.
// ------------------------------------------------------------
Position Environment::safeSpawn() {
    Position pos;
    int attempts = 0;
    const int MAX_ATTEMPTS = 1000;

    do {
        pos.row = rand() % ROWS;
        pos.col = rand() % COLS;
        attempts++;

        if (attempts > MAX_ATTEMPTS) {
            // Safety fallback: scan grid linearly for first EMPTY cell
            for (int r = 0; r < ROWS; r++) {
                for (int c = 0; c < COLS; c++) {
                    if (grid[r][c] == CellType::EMPTY) {
                        return {r, c};
                    }
                }
            }
            throw std::runtime_error("[Environment] safeSpawn: no empty cells available!");
        }

    } while (grid[pos.row][pos.col] != CellType::EMPTY);

    return pos;
}

// ------------------------------------------------------------
// addObstacle() / removeObstacle()
// Manual obstacle placement — for user-controlled editing
// or predefined map layouts.
// ------------------------------------------------------------
void Environment::addObstacle(int row, int col) {
    if (!isInBounds(row, col)) return;
    if (grid[row][col] == CellType::EMPTY) {
        grid[row][col] = CellType::OBSTACLE;
        obstacleCount++;
        std::cout << "[Environment] Obstacle added at (" << row << "," << col << ")\n";
    }
}

void Environment::removeObstacle(int row, int col) {
    if (!isInBounds(row, col)) return;
    if (grid[row][col] == CellType::OBSTACLE) {
        grid[row][col] = CellType::EMPTY;
        obstacleCount--;
        std::cout << "[Environment] Obstacle removed at (" << row << "," << col << ")\n";
    }
}

// ------------------------------------------------------------
// moveObstacle()
// Simulates dynamic obstacle behavior — obstacle shifts position.
// Used for moving obstacle extensions (future feature).
// ------------------------------------------------------------
void Environment::moveObstacle(Position from, Position to) {
    if (!isInBounds(from.row, from.col) || !isInBounds(to.row, to.col)) return;
    if (grid[from.row][from.col] != CellType::OBSTACLE) return;
    if (grid[to.row][to.col] != CellType::EMPTY) return;

    grid[from.row][from.col] = CellType::EMPTY;
    grid[to.row][to.col]     = CellType::OBSTACLE;

    std::cout << "[Environment] Obstacle moved from (" 
              << from.row << "," << from.col << ") to ("
              << to.row   << "," << to.col   << ")\n";
}

// ------------------------------------------------------------
// getObstacleCount()
// Utility for statistics display / density verification.
// ------------------------------------------------------------
int Environment::getObstacleCount() const {
    return obstacleCount;
}

// ------------------------------------------------------------
// displayGrid()
// Prints a colored ASCII map to the console for debugging.
// Symbols:
//   .  = empty
//   █  = obstacle
//   D  = drone
//   T  = target
//   F  = fuel station
// ------------------------------------------------------------
void Environment::displayGrid() const {
    std::cout << "\n  ";
    for (int c = 0; c < COLS; c++) std::cout << c << " ";
    std::cout << "\n";

    for (int r = 0; r < ROWS; r++) {
        std::cout << r << " ";
        for (int c = 0; c < COLS; c++) {
            switch (grid[r][c]) {
                case CellType::EMPTY:    std::cout << ". "; break;
                case CellType::OBSTACLE: std::cout << "# "; break;
                case CellType::DRONE:    std::cout << "D "; break;
                case CellType::TARGET:   std::cout << "T "; break;
                case CellType::FUEL:     std::cout << "F "; break;
                default:                 std::cout << "? "; break;
            }
        }
        std::cout << "\n";
    }
    std::cout << "[Obstacles: " << obstacleCount << "]\n\n";
}

// ------------------------------------------------------------
// IEnvironment interface implementations
// Coordinate convention: Point.x → col, Point.y → row
// (Pathfinder and AILogicController use Cartesian x/y;
//  Environment stores in matrix row/col order.)
// ------------------------------------------------------------

// isWalkable()
// Called by Pathfinder::getNeighbors() and AILogicController::handleTracking()
// to check whether a cell can be entered.
// Delegates to existing isValidMove() — no logic duplication.
bool Environment::isWalkable(int x, int y) const {
    return isValidMove(y, x);   // y → row, x → col
}

// isValid()
// Called by Pathfinder::getNeighbors() to prune out-of-bounds neighbors.
// isInBounds() is private, so we re-express it here using the public
// coordinate order rather than changing isInBounds() visibility.
bool Environment::isValid(int x, int y) const {
    return (y >= 0 && y < ROWS && x >= 0 && x < COLS); // y→row, x→col
}

// getFuelStations()
// Called by AILogicController::handleRefueling() via IEnvironment*.
// Scans the grid for every CellType::FUEL cell and returns their
// coordinates as Point values (x=col, y=row).
// This method fills the gap identified in the architecture analysis —
// Environment could place FUEL cells but had no way to query them.
std::vector<DroneSim::Point> Environment::getFuelStations() const {
    std::vector<DroneSim::Point> stations;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (grid[r][c] == CellType::FUEL) {
                stations.push_back({ c, r }); // x=col, y=row
            }
        }
    }
    return stations;
}