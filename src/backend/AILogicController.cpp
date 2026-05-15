#include "AILogicController.h"
#include <iostream>

namespace DroneSim {

    // Implementing the constructor
    AILogicController::AILogicController(const IEnvironment* e, const IDroneState* d) 
        : env(e), droneState(d), pathfinder(e), currentState(AIState::IDLE) {}

    // Implementing the main brain loop
    Point AILogicController::determineNextMove(Point currentTarget) {
        Point currentPos = droneState->getPosition();
        
        evaluateState(currentPos, currentTarget);

        switch (currentState) {
            case AIState::SEEKING_FUEL:
                return handleRefueling(currentPos);
            case AIState::TRACKING_TARGET:
                return handleTracking(currentPos, currentTarget);
            case AIState::IDLE:
            default:
                return currentPos; // Stay put
        }
    }

    // Implementing the state evaluator
    void AILogicController::evaluateState(Point currentPos, Point currentTarget) {
        float fuelPercent = (droneState->getFuelLevel() / droneState->getMaxFuel()) * 100.0f;

        if (fuelPercent <= CRITICAL_FUEL_THRESHOLD) {
            if (currentState != AIState::SEEKING_FUEL) {
                std::cout << "[AI Log] Critical fuel! Switching to SEEKING_FUEL state.\n";
                currentState = AIState::SEEKING_FUEL;
                currentPath.clear();
            }
        } else {
            currentState = AIState::TRACKING_TARGET;
        }
    }

    // Implementing refueling logic
    Point AILogicController::handleRefueling(Point currentPos) {
        if (currentPath.empty()) {
            std::vector<Point> stations = env->getFuelStations();
            if (stations.empty()) return currentPos;

            Point nearest = pathfinder.findNearestFuelStation(currentPos, stations);
            currentPath = pathfinder.findPathAStar(currentPos, nearest);
        }
        return extractNextStep(currentPos);
    }

    // Implementing tracking logic
    Point AILogicController::handleTracking(Point currentPos, Point currentTarget) {
        if (currentTarget != lastKnownTarget || currentPath.empty()) {
            currentPath = pathfinder.findPathAStar(currentPos, currentTarget);
            lastKnownTarget = currentTarget;
        }

        if (!currentPath.empty() && !env->isWalkable(currentPath.front().x, currentPath.front().y)) {
            std::cout << "[AI Log] Dynamic obstacle detected! Recalculating path.\n";
            currentPath = pathfinder.findPathAStar(currentPos, currentTarget);
        }

        return extractNextStep(currentPos);
    }

    // Implementing path extraction
    Point AILogicController::extractNextStep(Point currentPos) {
        if (!currentPath.empty()) {
            Point nextMove = currentPath.front();
            currentPath.erase(currentPath.begin()); 
            return nextMove;
        }
        return currentPos; 
    }

    // getCurrentState()
    // Read-only accessor for simulation manager/debug systems
    AIState AILogicController::getCurrentState() const {
        return currentState; 
    }
}