#pragma once
#include "SimInterfaces.h"
#include <vector>
#include <unordered_map>

namespace DroneSim {

    // Custom hash for unordered_map to store Points
    struct PointHash {
        std::size_t operator()(const Point& p) const;
    };

    class Pathfinder {
    private:
        const IEnvironment* env;

        // Private helper functions declared here
        int heuristic(Point a, Point b) const;
        std::vector<Point> getNeighbors(Point p) const;

    public:
        // Constructor and public functions declared here
        explicit Pathfinder(const IEnvironment* environment);

        std::vector<Point> findPathAStar(Point start, Point goal);
        Point findNearestFuelStation(Point start, const std::vector<Point>& stations);
    };

}