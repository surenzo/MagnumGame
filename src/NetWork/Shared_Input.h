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

    void addClickAction(InputAction action, Magnum::Vector2 position) {
        std::lock_guard<std::mutex> lock(_mutex);
        _inputActionsWithPosition.push_back({action, position});
    }

    void clearInputActions() {
        std::lock_guard<std::mutex> lock(_mutex);
        _inputActions.clear();
        _inputActionsWithPosition.clear();
    }

    std::pmr::list<InputAction> getInputActions() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _inputActions;
    }

    std::pmr::list<std::pair<InputAction, Magnum::Vector2>> getInputActionsWithPosition() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _inputActionsWithPosition;
    }
private:
    std::mutex _mutex;
    std::pmr::list<InputAction> _inputActions;
    std::pmr::list<std::pair<InputAction, Magnum::Vector2>> _inputActionsWithPosition;
};


