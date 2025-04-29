#ifndef MAIN_SERVER_H
#define MAIN_SERVER_H

#include "Magnum/Timeline.h"
#include "Network/Server.hpp"
#include "Network/Shared_Input.h"
#include "Network/Shared_Objects.h"
#include "physics/PhysicsSystem.hpp"
#include "ECS/Serialization.cpp"
#include "Magnum/GL/DefaultFramebuffer.h"
#include "httplib.h"
#include <nlohmann/json.hpp>

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
using namespace Math::Literals;

class ServerApplication {
public:
    ServerApplication(std::shared_ptr<Shared_Input> inputStates, std::shared_ptr<Shared_Objects> objectStates);
    void updateRegistry();
    std::tuple<int, std::vector<int>> loop();
    void tick();
    void reset();
private:
    RigidBody* createAndAddEntity(int player, Object3D* parent, ShapeComponent::ShapeType shapeType, const Vector3& sizeOrRadius, float mass, const Color3& color);
    void startGame();
    void PlayInputs();
    void decrement_cubes(entt::registry& registry, entt::entity entity);

    Timeline _timeline;
    entt::registry _registry;

    PhysicsSystem _physicSystem;
    std::vector<Object3D*> _cameraRig, _cameraObject;

    std::unordered_map<InputAction, std::function<void(int)>> actionHandlers;

    float entityID = 0;
    std::shared_ptr<Shared_Input> inputState;
    std::shared_ptr<Shared_Objects> objectState;
    std::vector<int> nbOfCubes;
};


#endif // MAIN_SERVER_H
