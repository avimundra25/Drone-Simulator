#!/usr/bin/env bash
set -e

echo "=== Drone Simulator — Build & Run ==="

# 1. Install dependency (Crow needs ASIO)
if ! dpkg -s libasio-dev &>/dev/null; then
  echo "[*] Installing libasio-dev..."
  sudo apt-get update -qq && sudo apt-get install -y libasio-dev
fi

# 2. Build
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ..

# 3. Run
echo ""
echo "[*] Server starting on ws://localhost:8080/ws"
echo "[*] Open src/frontend/DroneSimulator.html in your browser"
echo "[*] Click the canvas to set targets, use D-Pad or WASD to override"
echo ""
./build/DroneServer
