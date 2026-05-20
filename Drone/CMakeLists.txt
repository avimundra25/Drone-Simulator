cmake_minimum_required(VERSION 3.15)
project(DroneSim)

set(CMAKE_CXX_STANDARD 17)

# Find necessary threading for Crow
find_package(Threads REQUIRED)

# Assuming crow_all.h and json.hpp are in an /include directory
include_directories(include)

add_executable(DroneServer 
    src/backend/AILogicController.cpp
    src/backend/Environment.cpp
    src/backend/Pathfinder.cpp
    src/backend/DroneMovement.cpp
    src/backend/FuelManagement.cpp
    src/backend/SimulationController.cpp
    src/backend/Server.cpp
)

target_link_libraries(DroneServer Threads::Threads)