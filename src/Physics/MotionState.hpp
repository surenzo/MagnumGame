// Physics/MotionState.hpp
#pragma once
#include <btBulletDynamicsCommon.h>
#include "../ECS/Components.hpp"

struct MotionStateWrapper : public btMotionState {
    Objet3D* objet;

    MotionStateWrapper(Objet3D* obj) : objet(obj) {}

    void getWorldTransform(btTransform& worldTrans) const override {
        worldTrans.setOrigin(btVector3(objet->position.x(), objet->position.y(), objet->position.z()));
        worldTrans.setRotation(btQuaternion(objet->rotation.vector().x(), objet->rotation.vector().y(), objet->rotation.vector().z(), objet->rotation.scalar()));
    }

    void setWorldTransform(const btTransform& worldTrans) override {
        const auto& pos = worldTrans.getOrigin();
        const auto& rot = worldTrans.getRotation();
        objet->position = {pos.x(), pos.y(), pos.z()};
        objet->rotation = Magnum::Quaternion({rot.x(), rot.y(), rot.z()}, rot.w());
    }
};
