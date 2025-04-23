#pragma once
#include <btBulletDynamicsCommon.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include "../ECS/Components.hpp"
#include "Rigidbody.h"
#include "ColoredDrawable.h"


class PhysicsSystem {

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

public:
    PhysicsSystem();
    ~PhysicsSystem();

    void update(float dt);

    btDiscreteDynamicsWorld* getWorld() { return &_bWorld; }
    Scene3D* getScene() { return &_scene; }
    RigidBody* addRigidBody(float mass, bool isBox);
    RigidBody* addGround();

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

