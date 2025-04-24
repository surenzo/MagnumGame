#include <iostream>

#include "Magnum/Timeline.h"
#include "Network/Server.hpp"
#include "Network/Shared_Input.h"
#include "Network/Shared_Objects.h"
#include "physics/PhysicsSystem.hpp"

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
using namespace Math::Literals;
class ServerApplication {
public:
    ServerApplication(std::shared_ptr<Shared_Input> inputStates, std::shared_ptr<Shared_Objects> objectStates);
    void loop();
    void tick();
private:
    Timeline _timeline;

    PhysicsSystem _physicSystem;
    Object3D *_cameraRig, *_cameraObject;

    std::shared_ptr<Shared_Input> inputState = std::make_shared<Shared_Input>();
    std::shared_ptr<Shared_Objects> objectState = std::make_shared<Shared_Objects>();
};

ServerApplication::ServerApplication(std::shared_ptr<Shared_Input> inputStates, std::shared_ptr<Shared_Objects> objectStates) {

    inputState = inputStates;
    objectState = objectStates;
    (*(_cameraRig = new Object3D{_physicSystem.getScene()}))
        .translate(Vector3::yAxis(3.0f))
        .rotateY(40.0_degf);
    (*(_cameraObject = new Object3D{_cameraRig}))
        .translate(Vector3::zAxis(20.0f))
        .rotateX(-25.0_degf);
    auto* ground = _physicSystem.addBox({100.0f, 0.5f, 100.0f}, 0.0f);

    for(Int i = 0; i != 5; ++i) {
        for(Int j = 0; j != 5; ++j) {
            for(Int k = 0; k != 5; ++k) {
                auto* o = _physicSystem.addBox(Vector3{0.5f});
                o->translate({i - 2.0f, j + 4.0f, k - 2.0f});
                o->syncPose();
            }
        }
    }
    _timeline.start();
}

void ServerApplication::loop() {
    while (true) {
        tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // approx 60 FPS
    }
}

void ServerApplication::tick() {
    //play inputs
    auto inputActions = inputState->getInputActions();
    inputState->clearInputActions();

    if (_cameraObject == nullptr) {
        std::cerr << "Camera object is null" << std::endl;
        return;
    }
    std::unordered_map<InputAction, std::function<void()>> actionHandlers{
        {InputAction::FORWARD, [this]() { _cameraObject->translateLocal(Vector3::zAxis(-0.1f)); }},
        {InputAction::BACKWARD, [this]() { _cameraObject->translateLocal(Vector3::zAxis(0.1f)); }},
        {InputAction::LEFT, [this]() { _cameraObject->translateLocal(Vector3::xAxis(-0.1f)); }},
        {InputAction::RIGHT, [this]() { _cameraObject->translateLocal(Vector3::xAxis(0.1f)); }},
        {InputAction::UP, [this]() { _cameraObject->translateLocal(Vector3::yAxis(0.1f)); }},
        {InputAction::DOWN, [this]() { _cameraObject->translateLocal(Vector3::yAxis(-0.1f)); }},
        {InputAction::ROTATE_UP, [this]() { _cameraObject->rotateX(-5.0_degf); }},
        {InputAction::ROTATE_DOWN, [this]() { _cameraObject->rotateX(5.0_degf); }},
        {InputAction::ROTATE_LEFT, [this]() { _cameraObject->rotateY(-5.0_degf); }},
        {InputAction::ROTATE_RIGHT, [this]() { _cameraObject->rotateY(5.0_degf); }}
    };

    for (auto action : inputActions) {
        auto it = actionHandlers.find(action);
        if (it != actionHandlers.end()) {
            it->second(); // Execute the corresponding action
        }
    }

    //send the new camera position to the clients
    objectState->addCameraPosition(_cameraObject->absoluteTransformation().translation(),
                                   Quaternion::fromMatrix(_cameraObject->absoluteTransformation().rotation()));

    _physicSystem.update(_timeline.previousFrameDuration());
    // Here you can add code to send the state of the objects to the clients
    // For example, serialize the positions and rotations of the objects and send them over the network
    // You can use your serialization functions from Serialization.hpp
    _timeline.nextFrame();
}


int main() {
    std::shared_ptr<Shared_Input> inputStates = std::make_shared<Shared_Input>();
    std::shared_ptr<Shared_Objects> objectStates = std::make_shared<Shared_Objects>();
    Server server;
    ServerApplication app{inputStates, objectStates};
    if (!server.start(20000))
        return -1;

    server.run(inputStates, objectStates);
    app.loop();

    //server.stop();
    return 0;
}
