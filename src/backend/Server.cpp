#include "crow_all.h" // Assuming single-header Crow
#include "SimulationController.h"
#include <thread>
#include <chrono>
#include <unordered_set>

using namespace DroneSim;

int main() {
    crow::SimpleApp app;
    SimulationController sim;
    std::unordered_set<crow::websocket::connection*> users;
    std::mutex mtx;

    // WebSocket Endpoint
    CROW_WEBSOCKET_ROUTE(app, "/ws")
      .onopen([&](crow::websocket::connection& conn) {
          std::lock_guard<std::mutex> _(mtx);
          users.insert(&conn);
      })
      .onclose([&](crow::websocket::connection& conn, const std::string& reason) {
          std::lock_guard<std::mutex> _(mtx);
          users.erase(&conn);
      })
      .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
          auto payload = nlohmann::json::parse(data);
          std::string action = payload["action"];
          
          if (action == "START") sim.start();
          else if (action == "PAUSE") sim.pause();
          else if (action == "RESET") sim.reset();
          else if (action == "SET_TARGET") {
              sim.setTarget(payload["x"], payload["y"]);
          }
      });

    // Background Thread for Simulation Tick
    std::thread simThread([&]() {
        while (true) {
            sim.updateTick();
            
            // Broadcast state to all connected frontends
            auto stateJson = sim.getGameStateJson().dump();
            {
                std::lock_guard<std::mutex> _(mtx);
                for (auto u : users) {
                    u->send_text(stateJson);
                }
            }
            // Run at ~5 ticks per second for real-time visualization
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); 
        }
    });

    app.port(8080).multithreaded().run();
    simThread.join();
    return 0;
}