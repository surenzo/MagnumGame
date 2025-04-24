#pragma once
#include <mutex>

#include "Magnum/Math/Quaternion.h"
#include "Magnum/Math/Vector3.h"

using namespace Magnum;

class Shared_Objects {
public:

    Shared_Objects() = default;

    void addCameraPosition(Vector3 position, Quaternion rotation) {
        std::lock_guard<std::mutex> lock(_mutex);
        _cameraPosition.first = position;
        _cameraPosition.second = rotation;
    }
    std::pair<Vector3, Quaternion> getCameraPosition() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _cameraPosition;
    }
private:

    std::mutex _mutex;
    std::pair<Vector3, Quaternion> _cameraPosition;

};


