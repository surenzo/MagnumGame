#pragma once
#include <mutex>
#include <vector>

#include "Magnum/Math/Quaternion.h"
#include "Magnum/Math/Vector3.h"

using namespace Magnum;

class Shared_Objects {
public:

    Shared_Objects() = default;

    void setWorld(std::vector<uint8_t> newWorld) {
        std::lock_guard<std::mutex> lock(_mutex);
        world = std::move(newWorld);
    }
    std::vector<uint8_t> getWorld() {
        std::lock_guard<std::mutex> lock(_mutex);
        return world;
    }
    void setWinner(int newWinner) {
        std::lock_guard<std::mutex> lock(_mutex);
        winner = newWinner;
    }
    int getWinner() {
        std::lock_guard<std::mutex> lock(_mutex);
        return winner;
    }
private:

    std::mutex _mutex;
    std::vector<uint8_t> world;
    int winner = -1;

};


