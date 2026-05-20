#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <string>
using namespace std;

// ─────────────────────────────────────────
//  Constants
// ─────────────────────────────────────────
const float BATTERY_DRAIN_RATE    = 1.5f;
const float LOW_BATTERY_THRESHOLD = 25.0f;
const float FULL_BATTERY          = 100.0f;

// ─────────────────────────────────────────
//  Struct: FuelStation
// ─────────────────────────────────────────
struct FuelStation
{
    string name;
    float  x;
    float  y;
};

// ─────────────────────────────────────────
//  Class: Drone
// ─────────────────────────────────────────
class Drone
{
private:
    float x;
    float y;
    float fuel;
    vector<FuelStation> stations;

    // ── Helpers ──────────────────────────

    // Doesn't use 'this', so it's static
    static float calculateDistance(
        float x1, float y1,
        float x2, float y2)
    {
        return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
    }

    void decreaseFuel(float distance)
    {
        fuel -= distance * BATTERY_DRAIN_RATE;
        if (fuel < 0.0f)
            fuel = 0.0f;                   // clamp, no magic branch
    }

    void printMoveSummary(float distance) const
    {
        cout << "\n========================";
        cout << "\nDrone Moved";
        cout << "\n========================";
        cout << "\nCurrent Position : (" << x << ", " << y << ")";
        cout << "\nDistance Travelled: " << distance;
        cout << "\nFuel Remaining   : " << fuel << "%\n";
    }

    // Returns the nearest station (object, not just its name)
    FuelStation findNearestStation() const
    {
        FuelStation nearest;
        float minDist = numeric_limits<float>::max();   // no magic 99999

        for (const auto& station : stations)            // const& avoids copies
        {
            float dist = calculateDistance(x, y, station.x, station.y);
            if (dist < minDist)
            {
                minDist  = dist;
                nearest  = station;
            }
        }
        return nearest;
    }

    void checkFuel()
    {
        if (fuel <= LOW_BATTERY_THRESHOLD)
        {
            cout << "\nLOW FUEL WARNING!\n";
            FuelStation nearest = findNearestStation();
            cout << "Nearest Fuel Station : " << nearest.name << "\n";
            cout << "Distance to Station  : "
                 << calculateDistance(x, y, nearest.x, nearest.y)
                 << " km\n";
        }
        else
        {
            cout << "Fuel Status: SAFE\n";
        }
    }

public:
    // ── Constructor ──────────────────────
    Drone(float startX, float startY)
        : x(startX), y(startY), fuel(FULL_BATTERY)
    {
        stations.push_back({"Alpha Station", 5,  8 });
        stations.push_back({"Beta Station",  15, 3 });
        stations.push_back({"Gamma Station", 20, 15});
    }

    // ── Public interface ─────────────────

    void moveDrone(float newX, float newY)
    {
        // Guard: can't move on empty tank
        if (fuel <= 0.0f)
        {
            cout << "\nCannot move — fuel tank is empty!\n";
            return;
        }

        float distance = calculateDistance(x, y, newX, newY);
        decreaseFuel(distance);

        x = newX;
        y = newY;

        printMoveSummary(distance);
        checkFuel();
    }

    void refuel()
    {
        fuel = FULL_BATTERY;
        cout << "\nDrone Refuelled Successfully!\n";
    }

    void displayStatus() const
    {
        cout << "\n========================";
        cout << "\nDRONE STATUS";
        cout << "\n========================";
        cout << "\nPosition : (" << x << ", " << y << ")";
        cout << "\nFuel     : " << fuel << "%\n";
    }
};
