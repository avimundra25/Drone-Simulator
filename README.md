# Autonomous Drone AI Simulator

## 🚁 Project Overview
This project is a real-time autonomous drone simulation that demonstrates a **Logic-First** integration of a high-performance C++ backend with a dynamic web-based frontend. The drone navigates a 10x10 grid, avoids procedurally generated obstacles, and manages complex internal states like battery life and path optimization.

The simulation illustrates how low-latency C++ logic can power real-time robotic decision-making via WebSockets.

## 🧠 Core Features & AI Logic
* **A* Pathfinding Algorithm:** A custom C++ implementation of the A* algorithm allows the drone to calculate the most efficient route while navigating around blocked cells.
* **Finite State Machine (FSM):** The drone's "brain" is managed by a state machine:
    * `IDLE`: The drone is stationary, awaiting a target.
    * `MOVING`: The drone is actively following a calculated path.
    * `SEEKING_FUEL`: An emergency state triggered when battery drops below 20%, forcing the drone to prioritize survival over user commands.
* **WebSocket Telemetry:** High-frequency data exchange (10Hz) using the **Crow** framework, sending real-time coordinates and fuel percentages to the UI.
* **Procedural Generation:** Every simulation session generates a new set of environmental obstacles, testing the AI's adaptability.

## 🛠 Tech Stack
* **Backend:** C++17 (Logic, Pathfinding, State Management)
* **Server Framework:** Crow (C++ WebSockets)
* **Data Handling:** Nlohmann JSON for C++
* **Frontend:** HTML5 Canvas & JavaScript
* **Infrastructure:** Linux / GitHub Codespaces

## 📂 Project Structure
* `src/backend/`: Contains the core C++ logic (Pathfinder, AILogic, Environment).
* `include/`: Header files and external library definitions.
* `DroneSimulator.html`: The interactive frontend visualization.
* `CMakeLists.txt`: Build configuration for the C++ executable.

## 🚀 How to Run (Submission Instructions)
To run this simulation in a **GitHub Codespace**, follow these steps:

1.  **Install Dependencies:**
    Run this command in the terminal to install the networking library:
    ```bash
    sudo apt-get update && sudo apt-get install -y libasio-dev
    ```

2.  **Build the Project:**
    ```bash
    mkdir build && cd build
    cmake ..
    make
    ```

3.  **Launch the Server:**
    ```bash
    ./DroneServer
    ```

4.  **Connect the Frontend:**
    * Navigate to the **PORTS** tab in your Codespace.
    * Set the visibility of Port **8080** to **Public**.
    * Copy the **Forwarded Address** URL.
    * Open `DroneSimulator.html` on your local machine and update the WebSocket URL to: `wss://YOUR_COPIED_URL/ws`
    * Open the HTML file in any modern browser to see the live simulation.

---
**Developed for the C++ Mini-Project Submission.**
