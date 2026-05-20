#include "crow_all.h"
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
          // Send initial state immediately on connect
          conn.send_text(sim.getGameStateJson().dump());
      })
      .onclose([&](crow::websocket::connection& conn, const std::string&) {
          std::lock_guard<std::mutex> _(mtx);
          users.erase(&conn);
      })
      .onmessage([&](crow::websocket::connection& /*conn*/,
                     const std::string& data, bool /*is_binary*/) {
          try {
              auto payload = nlohmann::json::parse(data);
              std::string action = payload.value("action", "");

              if      (action == "START")  sim.start();
              else if (action == "PAUSE")  sim.pause();
              else if (action == "RESET")  sim.reset();
              else if (action == "SET_TARGET") {
                  sim.setTarget(payload["x"], payload["y"]);
              }
              else if (action == "MANUAL_MOVE") {
                  // D-pad / keyboard override: dx and dy are -1, 0, or +1
                  int dx = payload.value("dx", 0);
                  int dy = payload.value("dy", 0);
                  sim.manualMove(dx, dy);
              }
          } catch (const std::exception& e) {
              std::cerr << "[Server] Message parse error: " << e.what() << "\n";
          }
      });

    // Background thread: tick simulation at ~5 Hz and broadcast state
    std::thread simThread([&]() {
        while (true) {
            sim.updateTick();

            auto stateJson = sim.getGameStateJson().dump();
            {
                std::lock_guard<std::mutex> _(mtx);
                for (auto* u : users) {
                    u->send_text(stateJson);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    });
    simThread.detach(); // detach so app.run() blocking is fine

    app.port(8080).multithreaded().run();
    return 0;
}
