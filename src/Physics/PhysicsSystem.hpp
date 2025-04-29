#pragma once
#include <btBulletDynamicsCommon.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include "../ECS/Components.hpp"
#include "Rigidbody.h"


class PhysicsSystem {

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

public:
    PhysicsSystem();
    ~PhysicsSystem();

    std::vector<Object3D*> update(float dt);

    btDiscreteDynamicsWorld* getWorld() { return &_bWorld; }
    Scene3D* getScene() { return &_scene; }
    RigidBody* addBox(Vector3 size = {1.0f,1.0f,1.0f}, float mass = 1.0f);
    RigidBody* addSphere(float radius = 1.0f, float mass = 1.0f);
    RigidBody* addBox(Object3D* parent,Vector3 size = {1.0f,1.0f,1.0f}, float mass = 1.0f);
    RigidBody* addSphere(Object3D* parent, float radius = 1.0f, float mass = 1.0f);
    void reset();

private:
    btDbvtBroadphase _bBroadphase;
    btDefaultCollisionConfiguration _bCollisionConfig;
    btCollisionDispatcher _bDispatcher{&_bCollisionConfig};
    btSequentialImpulseConstraintSolver _bSolver;
    btDiscreteDynamicsWorld _bWorld{&_bDispatcher, &_bBroadphase, &_bSolver, &_bCollisionConfig};

    Scene3D _scene;

    btBoxShape _bBoxShape{{0.5f, 0.5f, 0.5f}};
    btSphereShape _bSphereShape{0.25f};
    btBoxShape _bGroundShape{{100.0f, 0.5f, 100.0f}};
};

