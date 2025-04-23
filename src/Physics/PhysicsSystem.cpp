#include "PhysicsSystem.hpp"
#include "MotionState.hpp"

PhysicsSystem::PhysicsSystem(entt::registry& reg) : registry(reg) {
    collisionConfig = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfig);
    broadphase = new btDbvtBroadphase();
    solver = new btSequentialImpulseConstraintSolver();
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
    dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
}

PhysicsSystem::~PhysicsSystem() {
    for (auto& [entity, body] : entityToBody) {
        dynamicsWorld->removeRigidBody(body);
        delete body->getMotionState();
        delete body->getCollisionShape();
        delete body;
    }
    delete dynamicsWorld;
    delete solver;
    delete broadphase;
    delete dispatcher;
    delete collisionConfig;
}

void PhysicsSystem::addRigidBody(entt::entity entity, float mass, bool isBox) {
    auto& obj = registry.get<Objet3D>(entity);

    btCollisionShape* shape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f)); // cube taille 1

    btVector3 localInertia(0, 0, 0);
    if(mass > 0.0f)
        shape->calculateLocalInertia(mass, localInertia);

    auto* motionState = new MotionStateWrapper(&obj);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);

    dynamicsWorld->addRigidBody(body);
    entityToBody[entity] = body;
}

void PhysicsSystem::stepSimulation(float deltaTime) {
    dynamicsWorld->stepSimulation(deltaTime);
}

void PhysicsSystem::syncTransformsToPhysics() {
    for (auto& [entity, body] : entityToBody) {
        if (registry.valid(entity)) {
            auto& obj = registry.get<Objet3D>(entity);
            btTransform transform;
            transform.setOrigin(btVector3(obj.position.x(), obj.position.y(), obj.position.z()));
            transform.setRotation(btQuaternion(obj.rotation.vector().x(), obj.rotation.vector().y(), obj.rotation.vector().z(), obj.rotation.scalar()));
            body->setWorldTransform(transform);
        }
    }
}

void PhysicsSystem::syncPhysicsToTransforms() {
    for (auto& [entity, body] : entityToBody) {
        if (registry.valid(entity)) {
            btTransform transform = body->getWorldTransform();
            auto& obj = registry.get<Objet3D>(entity);
            const btVector3& pos = transform.getOrigin();
            const btQuaternion& rot = transform.getRotation();
            obj.position = {pos.getX(), pos.getY(), pos.getZ()};
            obj.rotation = Magnum::Quaternion({rot.getX(), rot.getY(), rot.getZ()});
        }
    }
}
