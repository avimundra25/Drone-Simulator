#include "SimulationController.h"

namespace DroneSim {

    SimulationController::SimulationController() {
        env.initializeGrid();
        env.generateObstacles(15); // 15% density
        
        currentPos = {0, 0}; // Start at top left
        targetPos = {9, 9};  // Default target bottom right
        fuelLevel = MAX_FUEL;
        isRunning = false;

        // Initialize AI Controller with interfaces
        aiController = new AILogicController(&env, this);
    }

    SimulationController::~SimulationController() {
        delete aiController;
    }

    void SimulationController::start() { std::lock_guard<std::mutex> lock(simMutex); isRunning = true; }
    void SimulationController::pause() { std::lock_guard<std::mutex> lock(simMutex); isRunning = false; }
    
    void SimulationController::reset() {
        std::lock_guard<std::mutex> lock(simMutex);
        currentPos = {0, 0};
        fuelLevel = MAX_FUEL;
        env.initializeGrid();
        env.generateObstacles(15);
        isRunning = false;
    }

    void SimulationController::setTarget(int x, int y) {
        std::lock_guard<std::mutex> lock(simMutex);
        targetPos = {x, y};
    }

    void SimulationController::updateTick() {
        std::lock_guard<std::mutex> lock(simMutex);
        if (!isRunning || fuelLevel <= 0) return;

        // Let the AI dictate the next coordinate based on logic state
        Point nextMove = aiController->determineNextMove(targetPos);

        // Ensure move is valid in environment
        if (env.isValid(nextMove.x, nextMove.y) && env.isWalkable(nextMove.x, nextMove.y)) {
            // Calculate distance for fuel drain (from FuelManagement logic)
            float dist = std::sqrt(std::pow(nextMove.x - currentPos.x, 2) + std::pow(nextMove.y - currentPos.y, 2));
            if (dist > 0) {
                fuelLevel -= (dist * DRAIN_RATE);
                currentPos = nextMove;
            }
        }
    }

    nlohmann::json SimulationController::getGameStateJson() {
        std::lock_guard<std::mutex> lock(simMutex);
        nlohmann::json j;
        j["drone"]["x"] = currentPos.x;
        j["drone"]["y"] = currentPos.y;
        j["drone"]["fuel"] = fuelLevel;
        j["drone"]["state"] = static_cast<int>(aiController->getCurrentState());
        j["target"]["x"] = targetPos.x;
        j["target"]["y"] = targetPos.y;
        
        // Serialize obstacles for frontend rendering
        j["obstacles"] = nlohmann::json::array();
        for(int r=0; r<env.getRows(); r++){
            for(int c=0; c<env.getCols(); c++){
                if(env.isObstacle(r, c)){
                    j["obstacles"].push_back({{"x", c}, {"y", r}});
                }
            }
        }
        return j;
    }
}