#include "Pathfinder.h"
#include <queue>
#include <algorithm>
#include <cmath>

namespace DroneSim {

    // Implementing the PointHash struct
    std::size_t PointHash::operator()(const Point& p) const {
        return std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 1);
    }

    // Implementing the Constructor
    Pathfinder::Pathfinder(const IEnvironment* environment) : env(environment) {}

    // Implementing the private heuristic function
    int Pathfinder::heuristic(Point a, Point b) const {
        return std::abs(a.x - b.x) + std::abs(a.y - b.y);
    }

    // Implementing the neighbor search
    std::vector<Point> Pathfinder::getNeighbors(Point p) const {
        std::vector<Point> neighbors;
        int dx[] = {0, 1, 0, -1}; // Up, Right, Down, Left
        int dy[] = {-1, 0, 1, 0};
        
        for(int i=0; i<4; ++i) {
            Point next{p.x + dx[i], p.y + dy[i]};
            if (env->isValid(next.x, next.y) && env->isWalkable(next.x, next.y)) {
                neighbors.push_back(next);
            }
        }
        return neighbors;
    }

    // Implementing the A* Algorithm
    std::vector<Point> Pathfinder::findPathAStar(Point start, Point goal) {
        std::priority_queue<std::pair<int, Point>, 
                            std::vector<std::pair<int, Point>>, 
                            std::greater<std::pair<int, Point>>> frontier;
        
        std::unordered_map<Point, Point, PointHash> came_from;
        std::unordered_map<Point, int, PointHash> cost_so_far;

        frontier.push({0, start});
        came_from[start] = start;
        cost_so_far[start] = 0;

        bool found = false;

        while (!frontier.empty()) {
            Point current = frontier.top().second;
            frontier.pop();

            if (current == goal) {
                found = true;
                break;
            }

            for (Point next : getNeighbors(current)) {
                int new_cost = cost_so_far[current] + 1; // Assuming uniform grid cost
                
                if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next]) {
                    cost_so_far[next] = new_cost;
                    int priority = new_cost + heuristic(next, goal);
                    frontier.push({priority, next});
                    came_from[next] = current;
                }
            }
        }

        std::vector<Point> path;
        if (!found) return path;

        Point current = goal;
        while (current != start) {
            path.push_back(current);
            current = came_from[current];
        }
        std::reverse(path.begin(), path.end());
        return path;
    }
    
    // Implementing the nearest fuel station search
    Point Pathfinder::findNearestFuelStation(Point start, const std::vector<Point>& stations) {
        Point bestStation = start;
        int minCost = 1e9;

        for (const auto& station : stations) {
            auto path = findPathAStar(start, station);
            if (!path.empty() && path.size() < minCost) {
                minCost = path.size();
                bestStation = station;
            }
        }
        return bestStation;
    }

}