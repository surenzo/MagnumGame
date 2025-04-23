#pragma once
#include <btBulletDynamicsCommon.h>
#include <entt/entt.hpp>
#include "../ECS/Components.hpp"
#include "MotionState.hpp"

class PhysicsSystem {
public:
    PhysicsSystem(entt::registry& reg);
    ~PhysicsSystem();

    void stepSimulation(float deltaTime);
    void addRigidBody(entt::entity entity, float mass, bool isBox = true);
    void syncTransformsToPhysics();
    void syncPhysicsToTransforms();

private:
    entt::registry& registry;
    btDefaultCollisionConfiguration* collisionConfig;
    btCollisionDispatcher* dispatcher;
    btBroadphaseInterface* broadphase;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* dynamicsWorld;

    std::unordered_map<entt::entity, btRigidBody*> entityToBody;
};
