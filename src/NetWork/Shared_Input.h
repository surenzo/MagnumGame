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

    void addInputAction(InputAction action, int player) {
        std::lock_guard<std::mutex> lock(_mutex);
        _inputActionsWithPlayer.push_back({action, player});
    }


    void addClickAction(InputAction action, Magnum::Vector2 position) {
        std::lock_guard<std::mutex> lock(_mutex);
        _inputActionsWithPosition.push_back({action, position});
    }

    void addClickAction(InputAction action, Magnum::Vector2 position, int player) {
        std::lock_guard<std::mutex> lock(_mutex);
        _inputActionsWithPositionWithPlayer.push_back({action, position, player});
    }

    void clearInputActions() {
        std::lock_guard<std::mutex> lock(_mutex);
        _inputActions.clear();
        _inputActionsWithPosition.clear();
    }

    void clearInputActionsWithPlayer() {
        std::lock_guard<std::mutex> lock(_mutex);
        _inputActionsWithPlayer.clear();
        _inputActionsWithPositionWithPlayer.clear();
    }

    std::pmr::list<InputAction> getInputActions() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _inputActions;
    }

    std::pmr::list<std::pair<InputAction,int>> getInputActionsWithPlayer() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _inputActionsWithPlayer;
    }

    std::pmr::list<std::pair<InputAction, Magnum::Vector2>> getInputActionsWithPosition() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _inputActionsWithPosition;
    }

    std::pmr::list<std::tuple<InputAction, Magnum::Vector2,int>> getInputActionsWithPositionWithPlayer() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _inputActionsWithPositionWithPlayer;
    }
private:
    std::mutex _mutex;
    std::pmr::list<InputAction> _inputActions;
    std::pmr::list<std::pair<InputAction,int>> _inputActionsWithPlayer;
    std::pmr::list<std::pair<InputAction, Magnum::Vector2>> _inputActionsWithPosition;
    std::pmr::list<std::tuple<InputAction, Magnum::Vector2,int>> _inputActionsWithPositionWithPlayer;
};


