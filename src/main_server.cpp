#include <iostream>

#include "Magnum/Timeline.h"
#include "Network/Server.hpp"
#include "Network/Shared_Input.h"
#include "Network/Shared_Objects.h"
#include "physics/PhysicsSystem.hpp"
#include "ECS/Serialization.cpp"
#include "Magnum/GL/DefaultFramebuffer.h"

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
using namespace Math::Literals;
class ServerApplication {
public:
    ServerApplication(std::shared_ptr<Shared_Input> inputStates, std::shared_ptr<Shared_Objects> objectStates);
    void updateRegistry();
    void loop();
    void tick();
private:
    RigidBody* createAndAddEntity(Object3D* parent, ShapeComponent::ShapeType shapeType, const Vector3& sizeOrRadius, float mass, const Color3& color);
    void startGame();
    void PlayInputs();

    Timeline _timeline;
    entt::registry _registry;

    PhysicsSystem _physicSystem;
    std::vector<Object3D*> _cameraRig, _cameraObject;
    std::vector<Object3D*> places;

    std::unordered_map<InputAction, std::function<void()>> actionHandlers;

    float entityID = 0;
    std::shared_ptr<Shared_Input> inputState = std::make_shared<Shared_Input>();
    std::shared_ptr<Shared_Objects> objectState = std::make_shared<Shared_Objects>();
};

ServerApplication::ServerApplication(std::shared_ptr<Shared_Input> inputStates, std::shared_ptr<Shared_Objects> objectStates){
    _registry = entt::registry();
    inputState = inputStates;
    objectState = objectStates;
    // add new entity to the registry

    startGame();
}
void ServerApplication::startGame() {
    // Create 4 parent objects for the 4 players (platforms)
    std::vector<Object3D*> parents;
    for (int i = 0; i < 4; ++i) {
        // Dynamically allocate Object3D instances
        parents.emplace_back(new Object3D{_physicSystem.getScene()});
        parents[i]
            ->translate(Vector3::xAxis(20.0f * (i % 2)))
            .translate(Vector3::zAxis(20.0f * (i / 2)));
        Vector3 p = parents[i]->transformationMatrix().translation();
        std::cout << "Parent #" << i << " world pos = " << p.x() << ", " << p.y() << ", " << p.z() << std::endl;
        auto cameraEntity = _registry.create();

        _cameraRig.emplace_back(new Object3D{parents[i]});
        _cameraRig[i]
            ->translate(Vector3::yAxis(3.0f))
            .rotateY(40.0_degf);

        _cameraObject.emplace_back(new Object3D{_cameraRig[i]});
        _cameraObject[i]
            ->translate(Vector3::zAxis(20.0f))
            .rotateX(-25.0_degf);

        Matrix4 cameraTransform = _cameraObject[i]->transformationMatrix();
        Vector3 position = cameraTransform.translation();
        Quaternion rotation = Quaternion::fromMatrix(cameraTransform.rotation());

        _registry.emplace<TransformComponent>(cameraEntity, position, rotation);
        _registry.emplace<CameraComponent>(cameraEntity, /*id=*/i);
        _registry.emplace<CameraLinkComponent>(cameraEntity, _cameraObject[i]);

        auto ground = createAndAddEntity(parents[i], ShapeComponent::ShapeType::Box, {15.0f, 0.5f, 15.0f}, 0.0f, 0x220000_rgbf);

        places.emplace_back(ground);
        Deg hue = 42.0_degf;
        // Stacked boxes
        for (Int i = 0; i != 5; ++i) {
            for (Int j = 0; j != 5; ++j) {
                for (Int k = 0; k != 5; ++k) {
                    auto box = createAndAddEntity(
                        ground,
                        ShapeComponent::ShapeType::Box,
                        Vector3{1.0f},
                        1.0f,
                        Color3::fromHsv({hue += 137.5_degf, 0.75f, 0.9f}));
                    box->translate({i - 2.0f, j + 4.0f, k - 2.0f});
                    box->syncPose();
                }
            }
        }
    }

    std::unordered_map<InputAction, std::function<void()>> actionHandlers{
                {InputAction::FORWARD, [this]() { _cameraObject[0]->translateLocal(Vector3::zAxis(-.5f)); }},
                {InputAction::BACKWARD, [this]() { _cameraObject[0]->translateLocal(Vector3::zAxis(.5f)); }},
                {InputAction::LEFT, [this]() { _cameraObject[0]->translateLocal(Vector3::xAxis(-.5f)); }},
                {InputAction::RIGHT, [this]() { _cameraObject[0]->translateLocal(Vector3::xAxis(.5f)); }},
                {InputAction::UP, [this]() { _cameraObject[0]->translateLocal(Vector3::yAxis(.5f)); }},
                {InputAction::DOWN, [this]() { _cameraObject[0]->translateLocal(Vector3::yAxis(-.5f)); }},
                {InputAction::ROTATE_UP, [this]() { _cameraObject[0]->rotateX(-5.0_degf); }},
                {InputAction::ROTATE_DOWN, [this]() { _cameraObject[0]->rotateX(5.0_degf); }},
                {InputAction::ROTATE_LEFT, [this]() { _cameraObject[0]->rotateY(-5.0_degf); }},
                {InputAction::ROTATE_RIGHT, [this]() { _cameraObject[0]->rotateY(5.0_degf); }},
                   {InputAction::B, [this]() { /*printPlaces(places); printRegistery(_registry);*/ }},
            };
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

// void printRegistery(const entt::registry& _registry) {
//     //clear std::cout
//     //TODO : fonctionne que sur windows
//     system("cls");
//     auto view = _registry.view<TransformComponent, ShapeComponent, RenderComponent>();
//     for (auto entity : view) {
//         const auto& transform = view.get<TransformComponent>(entity);
//         const auto& shape = view.get<ShapeComponent>(entity);
//         const auto& render = view.get<RenderComponent>(entity);
//
//         std::cout << "Entity: " << static_cast<unsigned int>(entity) << "\n";
//         std::cout << "  Transform: " << transform.position.x() << ", " << transform.position.y() << ", " << transform.position.z() << "\n";
//         std::cout << "  Rotation: " << transform.rotation.xyzw().x() << ", " << transform.rotation.xyzw().y() << ", " << transform.rotation.xyzw().z() << ", " << transform.rotation.xyzw().w() << "\n";
//         std::cout << "  Shape: " << (shape.type == ShapeComponent::ShapeType::Sphere ? "Sphere" : "Box") << "\n";
//     }
// }
void ServerApplication::tick() {

    PlayInputs();
    updateRegistry();
    auto packet = serializeRegistry(_registry);
    objectState->setWorld(packet);


    std::vector<Object3D*> entitesToDestroy;
    _physicSystem.update(_timeline.previousFrameDuration(), entitesToDestroy);

    for (auto obj : entitesToDestroy) {
        // remove the object from the scene
        delete obj;
        // remove the object from the registry
        auto view = _registry.view<PhysicsLinkComponent>();
        for (auto entity : view) {
            if (view.get<PhysicsLinkComponent>(entity).body == obj) {
                _registry.destroy(entity);
                break;
            }
        }
        std::cout << "Object destroyed" << std::endl;
        std::flush(std::cout);
    }


    _timeline.nextFrame();
}

void printPlaces(std::vector<Object3D*> places) {
    for (Object3D* place : places) {
        Vector3 p = place->parent()->transformationMatrix().translation();
        std::cout << "Place position: " << p.x() << ", " << p.y() << ", " << p.z() << std::endl;
    }
}

void ServerApplication::PlayInputs() {
    //play inputs
    auto inputActions = inputState->getInputActions();
    auto inputActionsWithPosition = inputState->getInputActionsWithPosition();
    inputState->clearInputActions();



    for (auto action : inputActions) {
        auto it = actionHandlers.find(action);
        if (it != actionHandlers.end()) {
            it->second(); // Execute the corresponding action
        }
    }

    //do this if there is a click :

    for (auto action : inputActionsWithPosition) {
        if (action.first == InputAction::MOUSE_LEFT) {
            const Vector2 position = action.second;
            const Vector3 direction = (_cameraObject[0]->absoluteTransformation().rotationScaling() * Vector3{position, -1.0f}).normalized();

            auto* sphere = createAndAddEntity(_physicSystem.getScene(),ShapeComponent::ShapeType::Sphere, Vector3{1.0f}, 1.0f, 0x220000_rgbf);
            sphere->translate(_cameraObject[0]->absoluteTransformation().translation());
            sphere->rigidBody().setLinearVelocity(btVector3{direction * 25.f});
            sphere->syncPose();
        }
    }
}



RigidBody* ServerApplication::createAndAddEntity(
    Object3D* parent,
    ShapeComponent::ShapeType shapeType,
    const Vector3& sizeOrRadius,
    float mass,
    const Color3& color
) {
    RigidBody* body = nullptr;

    // Create the physics object based on the shape type
    if (shapeType == ShapeComponent::ShapeType::Sphere) {
        body = _physicSystem.addSphere(parent, sizeOrRadius.x(), mass);
    } else if (shapeType == ShapeComponent::ShapeType::Box) {
        body = _physicSystem.addBox(parent, sizeOrRadius, mass);
    }

    if (!body) {
        std::cerr << "Failed to create physics body for entity!" << std::endl;
        return nullptr;
    }

    // Create the entity in the registry
    auto entity = _registry.create();

    // Add components to the entity
    _registry.emplace<TransformComponent>(
        entity,
        body->transformationMatrix().translation(),
        Quaternion::fromMatrix(body->transformationMatrix().rotation())
    );

    _registry.emplace<ShapeComponent>(
        entity,
        shapeType,
        mass,
        shapeType == ShapeComponent::ShapeType::Box ? sizeOrRadius : Vector3{0.0f}, // size for boxes
        shapeType == ShapeComponent::ShapeType::Sphere ? sizeOrRadius.x() : 0.0f    // radius for spheres
    );

    _registry.emplace<RenderComponent>(
        entity,
        color,
        entityID++ // ID of the entity for rendering
    );

    _registry.emplace<PhysicsLinkComponent>(
        entity,
        body
    );

    return body;
}


int main(int argc, char** argv) {
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
