#include "PhysicsSystem.hpp"
#include "MotionState.hpp"

PhysicsSystem::PhysicsSystem()  {
    //auto* ground = new RigidBody{&_scene, 0.0f, &_bGroundShape, _bWorld};
}

PhysicsSystem::~PhysicsSystem() {

}
// TODO : add position and rotation to the rigid body
RigidBody* PhysicsSystem::addBox(Vector3 size, float mass) {
    btCollisionShape* shape = new btBoxShape(btVector3(size)); // cube taille 1
    auto* o = new RigidBody{&_scene, mass, shape, _bWorld};


    // TODO : mettre a jour la position o->translate({i - 2.0f, j + 4.0f, k - 2.0f});

    o->syncPose();
    return o;

// TODO : je sais pas ?

//    auto& obj = registry.get<Objet3D>(entity);
//
//    btCollisionShape* shape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f)); // cube taille 1
//
//    btVector3 localInertia(0, 0, 0);
//    if(mass > 0.0f)
//        shape->calculateLocalInertia(mass, localInertia);
//
//    auto* motionState = new MotionStateWrapper(&obj);
//
//    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
//    btRigidBody* body = new btRigidBody(rbInfo);
//
//    dynamicsWorld->addRigidBody(body);
//    entityToBody[entity] = body;
}

// RigidBody* PhysicsSystem::addGround(){
//     auto* ground = new RigidBody{&_scene, 0.0f, &_bGroundShape, _bWorld};
//     return ground;
//   }
RigidBody* PhysicsSystem::addSphere(float radius, float mass) {
    btCollisionShape* shape = new btSphereShape(radius);
    auto* o = new RigidBody{&_scene, mass, shape, _bWorld};


    // TODO : mettre a jour la position o->translate({i - 2.0f, j + 4.0f, k - 2.0f});

    o->syncPose();
    return o;
}
void PhysicsSystem::update(float dt){

    for(Object3D* obj = _scene.children().first(); obj; ) {
        Object3D* next = obj->nextSibling();
        if(obj->transformation().translation().dot() > 100*100)
            delete obj;
        obj = next;
    }

    _bWorld.stepSimulation(dt, 5);
  }



// TODO : add all this for entt
//void PhysicsSystem::syncTransformsToPhysics() {
//    for (auto& [entity, body] : entityToBody) {
//        if (registry.valid(entity)) {
//            auto& obj = registry.get<Objet3D>(entity);
//            btTransform transform;
//            transform.setOrigin(btVector3(obj.position.x(), obj.position.y(), obj.position.z()));
//            transform.setRotation(btQuaternion(obj.rotation.vector().x(), obj.rotation.vector().y(), obj.rotation.vector().z(), obj.rotation.scalar()));
//            body->setWorldTransform(transform);
//        }
//    }
//}
//
//void PhysicsSystem::syncPhysicsToTransforms() {
//    for (auto& [entity, body] : entityToBody) {
//        if (registry.valid(entity)) {
//            btTransform transform = body->getWorldTransform();
//            auto& obj = registry.get<Objet3D>(entity);
//            const btVector3& pos = transform.getOrigin();
//            const btQuaternion& rot = transform.getRotation();
//            obj.position = {pos.getX(), pos.getY(), pos.getZ()};
//            obj.rotation = Magnum::Quaternion({rot.getX(), rot.getY(), rot.getZ()});
//        }
//    }
//}
