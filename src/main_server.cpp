#include <iostream>

#include "Magnum/Timeline.h"
#include "Network/Server.hpp"
#include "Network/Shared_Input.h"
#include "Network/Shared_Objects.h"
#include "physics/PhysicsSystem.hpp"
#include "ECS/Serialization.cpp"

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
using namespace Math::Literals;
class ServerApplication {
public:
    ServerApplication(std::shared_ptr<Shared_Input> inputStates, std::shared_ptr<Shared_Objects> objectStates);
    void updateRegistry();
    void loop();
    void tick();
private:
    Timeline _timeline;
    entt::registry _registry;

    PhysicsSystem _physicSystem;
    Object3D *_cameraRig, *_cameraObject;

    std::shared_ptr<Shared_Input> inputState = std::make_shared<Shared_Input>();
    std::shared_ptr<Shared_Objects> objectState = std::make_shared<Shared_Objects>();
};

ServerApplication::ServerApplication(std::shared_ptr<Shared_Input> inputStates, std::shared_ptr<Shared_Objects> objectStates) {
    _registry = entt::registry();
    inputState = inputStates;
    objectState = objectStates;
    // add new entity to the registry

    auto cameraEntity = _registry.create();
    // add a transform component to the entity
    // add a camera component to the entity

    (*(_cameraRig = new Object3D{_physicSystem.getScene()}))
        .translate(Vector3::yAxis(3.0f))
        .rotateY(40.0_degf);
    (*(_cameraObject = new Object3D{_cameraRig}))
        .translate(Vector3::zAxis(20.0f))
        .rotateX(-25.0_degf);

    // Récupère la transformation globale (translation + rotation)
    Matrix4 globalTransform = _cameraObject->transformationMatrix();
    Vector3 position = globalTransform.translation();
    Quaternion rotation = Quaternion::fromMatrix(globalTransform.rotation());

    // Ajoute les composants
    _registry.emplace<TransformComponent>(cameraEntity, position, rotation);
    _registry.emplace<CameraComponent>(cameraEntity, /*id=*/0); // par exemple joueur 0
    _registry.emplace<CameraLinkComponent>(cameraEntity, _cameraObject);

    // 1. Sol
    {
        auto* ground = _physicSystem.addBox({20.0f, 0.5f, 20.0f}, 0.0f);

        auto groundEntity = _registry.create();

        _registry.emplace<TransformComponent>(
            groundEntity,
            ground->transformationMatrix().translation(),
            Quaternion::fromMatrix(ground->transformationMatrix().rotation())
        );

        _registry.emplace<ShapeComponent>(
            groundEntity,
            ShapeComponent::ShapeType::Box,
            0.0f, // masse
            Vector3{20.0f, 0.5f, 20.0f},
            0.0f
        );

        _registry.emplace<RenderComponent>(
            groundEntity,
            Color3{0.2f, 0.2f, 0.2f}
        );

        _registry.emplace<PhysicsLinkComponent>(
            groundEntity,
            ground
        );
    }

    Deg hue = 42.0_degf;
    // 2. Boîtes empilées
    for(Int i = 0; i != 5; ++i) {
        for(Int j = 0; j != 5; ++j) {
            for(Int k = 0; k != 5; ++k) {
                auto* box = _physicSystem.addBox(Vector3{0.5f});
                box->translate({i - 2.0f, j + 4.0f, k - 2.0f});
                box->syncPose();

                auto boxEntity = _registry.create();

                _registry.emplace<TransformComponent>(
                    boxEntity,
                    box->transformationMatrix().translation(),
                    Quaternion::fromMatrix(box->transformationMatrix().rotation())
                );

                _registry.emplace<ShapeComponent>(
                    boxEntity,
                    ShapeComponent::ShapeType::Box,
                    1.0f, // masse
                    Vector3{0.5f}, // taille
                    0.0f           // radius ignoré
                );

                _registry.emplace<RenderComponent>(
                    boxEntity,
                    Color3::fromHsv({hue += 137.5_degf, 0.75f, 0.9f})
                );

                _registry.emplace<PhysicsLinkComponent>(
                    boxEntity,
                    box
                );
            }
        }
    }
    _timeline.start();
}

void ServerApplication::updateRegistry() {
    // Sync des objets physiques (rigidbodies)
    auto physicsView = _registry.view<TransformComponent, PhysicsLinkComponent>();
    for (auto entity : physicsView) {
        auto& transform = physicsView.get<TransformComponent>(entity);
        auto* body = physicsView.get<PhysicsLinkComponent>(entity).body;

        const auto& matrix = body->transformationMatrix();
        transform.position = matrix.translation();
        transform.rotation = Magnum::Quaternion::fromMatrix(matrix.rotation());
    }

    // Sync des caméras attachées à des objets 3D
    auto cameraView = _registry.view<TransformComponent, CameraLinkComponent>();
    for (auto entity : cameraView) {
        auto& transform = cameraView.get<TransformComponent>(entity);
        auto* cameraObject = cameraView.get<CameraLinkComponent>(entity).cameraObject;

        const auto& matrix = cameraObject->transformation();
        transform.position = matrix.translation();
        transform.rotation = Magnum::Quaternion::fromMatrix(matrix.rotation());
    }
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
    updateRegistry();
    auto packet = serializeRegistry(_registry);
    objectState->setWorld(packet);

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
