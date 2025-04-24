// Utils/Input.hpp
#pragma once
#include <Magnum/Platform/GlfwApplication.h>

enum class InputAction {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
    ROTATE_LEFT,
    ROTATE_RIGHT,
    ROTATE_UP,
    ROTATE_DOWN,
};


struct InputState {
    bool forward = false, left = false, back = false, right = false;
};

inline void updateInput(const Magnum::Platform::Application::KeyEvent& e, InputState& state, bool down) {
    switch(e.key()) {
        case Magnum::Platform::GlfwApplication::Key::W: state.forward = down; break;
        case Magnum::Platform::GlfwApplication::Key::A: state.left = down; break;
        case Magnum::Platform::GlfwApplication::Key::S: state.back = down; break;
        case Magnum::Platform::GlfwApplication::Key::D: state.right = down; break;
        default: break;
    }
}
