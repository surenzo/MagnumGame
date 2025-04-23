// ECS/Components.hpp
#pragma once
#include <Magnum/Math/Vector3.h>
#include <Magnum/Math/Quaternion.h>
#include <btBulletDynamicsCommon.h>

struct Objet3D {
    Magnum::Vector3 position;
    Magnum::Quaternion rotation;
    btRigidBody* rigidbody = nullptr; // Lien vers le rigidbody dans Bullet
};
