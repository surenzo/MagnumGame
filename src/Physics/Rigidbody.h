#pragma once
#include <Corrade/Containers/Pointer.h>
#include <btBulletDynamicsCommon.h>
#include <Magnum/BulletIntegration/Integration.h>
#include <Magnum/BulletIntegration/MotionState.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>

using namespace Magnum;

class RigidBody: public SceneGraph::Object<SceneGraph::MatrixTransformation3D> {
public:
    RigidBody(SceneGraph::Object<SceneGraph::MatrixTransformation3D>* parent, Float mass, btCollisionShape* bShape, btDynamicsWorld& bWorld)
        : SceneGraph::Object<SceneGraph::MatrixTransformation3D>{parent}, _bWorld(bWorld) {
        btVector3 bInertia(0.0f, 0.0f, 0.0f);
        if(!Math::TypeTraits<Float>::equals(mass, 0.0f))
            bShape->calculateLocalInertia(mass, bInertia);

        auto* motionState = new BulletIntegration::MotionState{*this};
        _bRigidBody.emplace(btRigidBody::btRigidBodyConstructionInfo{
            mass, &motionState->btMotionState(), bShape, bInertia});
        _bRigidBody->forceActivationState(DISABLE_DEACTIVATION);
        bWorld.addRigidBody(_bRigidBody.get());
    }

    ~RigidBody() {
        _bWorld.removeRigidBody(_bRigidBody.get());
    }

    btRigidBody& rigidBody() { return *_bRigidBody; }

    void syncPose() {
        _bRigidBody->setWorldTransform(btTransform(transformationMatrix()));
    }
    btRigidBody& getRigidBody() {
        return *_bRigidBody;
    }

private:
    btDynamicsWorld& _bWorld;
    Containers::Pointer<btRigidBody> _bRigidBody;
};

