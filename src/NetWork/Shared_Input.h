#pragma once
#include <list>
#include <mutex>

#include "../Utils/Input.hpp"
class Shared_Input {
public:
    Shared_Input() = default;

    void addInputAction(InputAction action) {
        std::lock_guard<std::mutex> lock(_mutex);
        _inputActions.push_back(action);
    }

    void clearInputActions() {
        std::lock_guard<std::mutex> lock(_mutex);
        _inputActions.clear();
    }

    std::pmr::list<InputAction> getInputActions() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _inputActions;
    }
private:
    std::mutex _mutex;
    std::pmr::list<InputAction> _inputActions;
};


