#include "SimulationController.h"

namespace DroneSim {

    SimulationController::SimulationController() {
        env.initializeGrid();
        env.generateObstacles(15); // 15% density

        // Guarantee start & default target cells are always walkable
        env.removeObstacle(0, 0); // drone start  (x=0, y=0  →  col=0, row=0)
        env.removeObstacle(9, 9); // default target (x=9, y=9 →  col=9, row=9)

        currentPos = {0, 0};
        targetPos  = {9, 9};
        fuelLevel  = MAX_FUEL;
        isRunning  = false;

        placeFuelStations(); // spawn 3 fuel depots on empty cells

        aiController = new AILogicController(&env, this);
    }

    SimulationController::~SimulationController() {
        delete aiController;
    }

    // ── Private helper: place 3 FUEL cells at safe random positions ──
    void SimulationController::placeFuelStations() {
        for (int i = 0; i < 3; i++) {
            try {
                Position p = env.safeSpawn();
                env.placeObject(p.row, p.col, CellType::FUEL);
            } catch (...) {
                // Grid too full — skip gracefully
            }
        }
    }

    void SimulationController::start()  { std::lock_guard<std::mutex> lock(simMutex); isRunning = true;  }
    void SimulationController::pause()  { std::lock_guard<std::mutex> lock(simMutex); isRunning = false; }

    void SimulationController::reset() {
        std::lock_guard<std::mutex> lock(simMutex);
        currentPos = {0, 0};
        targetPos  = {9, 9};
        fuelLevel  = MAX_FUEL;
        isRunning  = false;

        env.initializeGrid();
        env.generateObstacles(15);
        env.removeObstacle(0, 0);
        env.removeObstacle(9, 9);
        placeFuelStations();

        // Re-create AI controller so FSM state & cached path are reset
        delete aiController;
        aiController = new AILogicController(&env, this);
    }

    void SimulationController::setTarget(int x, int y) {
        std::lock_guard<std::mutex> lock(simMutex);
        // Only accept valid, walkable targets
        if (env.isValid(x, y) && env.isWalkable(x, y)) {
            targetPos = {x, y};
        }
    }

    // ── Manual override: move drone one step in given direction ─────
    void SimulationController::manualMove(int dx, int dy) {
        std::lock_guard<std::mutex> lock(simMutex);
        if (fuelLevel <= 0) return;

        Point next = { currentPos.x + dx, currentPos.y + dy };
        if (env.isValid(next.x, next.y) && env.isWalkable(next.x, next.y)) {
            fuelLevel -= DRAIN_RATE;
            if (fuelLevel < 0.0f) fuelLevel = 0.0f;
            currentPos = next;
        }
    }

    void SimulationController::updateTick() {
        std::lock_guard<std::mutex> lock(simMutex);
        if (!isRunning || fuelLevel <= 0) return;

        Point nextMove = aiController->determineNextMove(targetPos);

        if (env.isValid(nextMove.x, nextMove.y) && env.isWalkable(nextMove.x, nextMove.y)) {
            float dist = std::sqrt(
                std::pow(nextMove.x - currentPos.x, 2.0f) +
                std::pow(nextMove.y - currentPos.y, 2.0f)
            );
            if (dist > 0) {
                fuelLevel -= (dist * DRAIN_RATE);
                if (fuelLevel < 0.0f) fuelLevel = 0.0f;
                currentPos = nextMove;
            }
        }

        // Auto-refuel when landing on a fuel station
        for (const auto& s : env.getFuelStations()) {
            if (s.x == currentPos.x && s.y == currentPos.y) {
                fuelLevel = MAX_FUEL;
                break;
            }
        }
    }

    nlohmann::json SimulationController::getGameStateJson() {
        std::lock_guard<std::mutex> lock(simMutex);

        nlohmann::json j;
        j["drone"]["x"]     = currentPos.x;
        j["drone"]["y"]     = currentPos.y;
        j["drone"]["fuel"]  = fuelLevel;
        j["drone"]["state"] = static_cast<int>(aiController->getCurrentState());
        j["target"]["x"]    = targetPos.x;
        j["target"]["y"]    = targetPos.y;
        j["isRunning"]      = isRunning;

        // Obstacles (matrix row/col → Cartesian x/y for frontend)
        j["obstacles"] = nlohmann::json::array();
        for (int r = 0; r < env.getRows(); r++) {
            for (int c = 0; c < env.getCols(); c++) {
                if (env.isObstacle(r, c)) {
                    j["obstacles"].push_back({{"x", c}, {"y", r}});
                }
            }
        }

        // Fuel stations (returned already as {x=col, y=row} by getFuelStations)
        j["fuelStations"] = nlohmann::json::array();
        for (const auto& s : env.getFuelStations()) {
            j["fuelStations"].push_back({{"x", s.x}, {"y", s.y}});
        }

        return j;
    }

} // namespace DroneSim
