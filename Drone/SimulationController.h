#pragma once
#include "SimInterfaces.h"
#include "Environment.h"
#include "Pathfinder.h"
#include "AILogicController.h"
#include <mutex>
#include <nlohmann/json.hpp> // Standard JSON library

namespace DroneSim {

    class SimulationController : public IDroneState {
    private:
        Environment env;
        AILogicController* aiController;
        
        // Drone State Variables
        Point currentPos;
        Point targetPos;
        float fuelLevel;
        const float MAX_FUEL = 100.0f;
        const float DRAIN_RATE = 1.5f; // Imported from FuelManagement.cpp logic

        std::mutex simMutex;
        bool isRunning;

        void placeFuelStations(); // helper: spawn fuel stations after grid init

    public:
        SimulationController();
        ~SimulationController();

        // Implement IDroneState
        Point getPosition() const override { return currentPos; }
        float getFuelLevel() const override { return fuelLevel; }
        float getMaxFuel() const override { return MAX_FUEL; }
        float getFuelConsumptionRate() const override { return DRAIN_RATE; }

        // Simulation Controls
        void start();
        void pause();
        void reset();
        void setTarget(int x, int y);
        void manualMove(int dx, int dy);

        // Core Tick Update
        void updateTick();

        // Serialization for Frontend
        nlohmann::json getGameStateJson();
    };
}