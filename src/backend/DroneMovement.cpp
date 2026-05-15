#include <iostream>
#include <vector>
#include <string>
using namespace std;

// ── Constants ────────────────────────────────────────────────
const int GRID_W = 10;
const int GRID_H = 10;

#define EMPTY    '.'
#define DRONE    'D'
#define OBSTACLE 'X'
#define TARGET   'T'

// ── Drone Class ──────────────────────────────────────────────
class Drone {

    public:
        int x, y;
        int fuel;
        int speed;
        string direction;
        bool alive;

        Drone(int startX, int startY, int startFuel) {
            x         = startX;
            y         = startY;
            fuel      = startFuel;
            speed     = 1;
            direction = "IDLE";
            alive     = true;
        }

        bool moveUp() {
            if (!canMove()) return false;
            if (y + speed >= GRID_H) { cout << "[BLOCKED] Top wall!\n"; return false; }
            y += speed; direction = "UP"; fuel--;
            return true;
        }

        bool moveDown() {
            if (!canMove()) return false;
            if (y - speed < 0) { cout << "[BLOCKED] Bottom wall!\n"; return false; }
            y -= speed; direction = "DOWN"; fuel--;
            return true;
        }

        bool moveLeft() {
            if (!canMove()) return false;
            if (x - speed < 0) { cout << "[BLOCKED] Left wall!\n"; return false; }
            x -= speed; direction = "LEFT"; fuel--;
            return true;
        }

        bool moveRight() {
            if (!canMove()) return false;
            if (x + speed >= GRID_W) { cout << "[BLOCKED] Right wall!\n"; return false; }
            x += speed; direction = "RIGHT"; fuel--;
            return true;
        }

        void printSensors() {
            cout << "Sensors → UP:"   << (GRID_H - 1) - y
                 << "  DOWN:"         << y
                 << "  LEFT:"         << x
                 << "  RIGHT:"        << (GRID_W - 1) - x << "\n";
        }

        void printStatus() {
            cout << "\n--- Drone Status ---\n";
            cout << "Position  : (" << x << ", " << y << ")\n";
            cout << "Direction : "  << direction         << "\n";
            cout << "Speed     : "  << speed             << "\n";
            cout << "Fuel      : "  << fuel              << "\n";
            cout << "--------------------\n";
        }

    private:
        bool canMove() {
            if (!alive)    { cout << "[ERROR] Drone is inactive!\n"; return false; }
            if (fuel <= 0) { cout << "[ERROR] Out of fuel!\n"; alive = false; return false; }
            return true;
        }
};

// ── Grid Class ───────────────────────────────────────────────
class Grid {

    public:
        char cells[GRID_H][GRID_W];
        int  targetX, targetY;

        Grid() {
            for (int row = 0; row < GRID_H; row++)
                for (int col = 0; col < GRID_W; col++)
                    cells[row][col] = EMPTY;
            targetX = -1;
            targetY = -1;
        }

        void placeObstacle(int x, int y) {
            if (inBounds(x, y)) cells[y][x] = OBSTACLE;
        }

        void setTarget(int x, int y) {
            if (inBounds(x, y)) {
                targetX = x; targetY = y;
                cells[y][x] = TARGET;
            }
        }

        bool isObstacle(int x, int y) {
            return inBounds(x, y) && cells[y][x] == OBSTACLE;
        }

        bool isTarget(int x, int y) {
            return x == targetX && y == targetY;
        }

        bool inBounds(int x, int y) {
            return x >= 0 && x < GRID_W && y >= 0 && y < GRID_H;
        }

        void render(Drone& d) {
            cout << "\n";
            for (int row = GRID_H - 1; row >= 0; row--) {
                cout << row << " | ";
                for (int col = 0; col < GRID_W; col++) {
                    if (d.x == col && d.y == row) cout << "D ";
                    else                           cout << cells[row][col] << " ";
                }
                cout << "\n";
            }
            cout << "    ";
            for (int col = 0; col < GRID_W; col++) cout << col << " ";
            cout << "\n\n";
        }
};

// ── Simulation Class ─────────────────────────────────────────
class Simulation {

    public:
        Drone drone;
        Grid  grid;
        int   moves;

        Simulation() : drone(0, 0, 30), moves(0) {}

        void move(string dir) {
            if (!drone.alive) {
                cout << "[INFO] Simulation over. Restart to play again.\n";
                return;
            }

            int oldX = drone.x;
            int oldY = drone.y;
            bool moved = false;

            if      (dir == "W" || dir == "UP")    moved = drone.moveUp();
            else if (dir == "S" || dir == "DOWN")  moved = drone.moveDown();
            else if (dir == "A" || dir == "LEFT")  moved = drone.moveLeft();
            else if (dir == "D" || dir == "RIGHT") moved = drone.moveRight();
            else {
                cout << "[ERROR] Unknown command: " << dir << "\n";
                cout << "        Use W / A / S / D  or  UP / DOWN / LEFT / RIGHT\n";
                return;
            }

            if (!moved) return;

            // obstacle collision
            if (grid.isObstacle(drone.x, drone.y)) {
                cout << "[CRASH] Hit obstacle at ("
                     << drone.x << ", " << drone.y << ")!\n";
                drone.alive = false;
                drone.x = oldX;
                drone.y = oldY;
                return;
            }

            moves++;
            cout << "Moved " << dir << " → ("
                 << drone.x << ", " << drone.y
                 << ")  Fuel: " << drone.fuel << "\n";

            drone.printSensors();
            grid.render(drone);

            // target check
            if (grid.isTarget(drone.x, drone.y)) {
                cout << "*** TARGET REACHED in " << moves << " moves! ***\n";
                drone.alive = false;
            }
        }

        void printHelp() {
            cout << "\nCommands:\n";
            cout << "  W or UP    → move up\n";
            cout << "  S or DOWN  → move down\n";
            cout << "  A or LEFT  → move left\n";
            cout << "  D or RIGHT → move right\n";
            cout << "  STATUS     → print drone info\n";
            cout << "  QUIT       → exit\n\n";
        }

        void run() {
            // setup world
            grid.placeObstacle(3, 3);
            grid.placeObstacle(3, 4);
            grid.placeObstacle(5, 6);
            grid.setTarget(8, 8);

            cout << "=== DRONE SIMULATOR ===\n";
            printHelp();
            drone.printStatus();
            grid.render(drone);

            // ── user input loop ───────────────────────────────
            string input;
            while (drone.alive) {
                cout << "Enter command: ";
                cin  >> input;

                // convert to uppercase
                for (char& c : input) c = toupper(c);

                if (input == "QUIT") {
                    cout << "Simulation ended.\n";
                    break;
                }
                else if (input == "STATUS") {
                    drone.printStatus();
                }
                else {
                    move(input);
                }
            }

            cout << "\n=== FINAL RESULT ===\n";
            drone.printStatus();
        }
};