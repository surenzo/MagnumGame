// Network/Serialization.hpp
#pragma once
#include <entt/entt.hpp>
#include <Magnum/Math/Vector3.h>
#include <Magnum/Math/Quaternion.h>
#include "../ECS/Components.hpp"
#include <cstring>

inline void serialize(const Objet3D& obj, char* buffer) {
    std::memcpy(buffer, &obj.position, sizeof(Magnum::Vector3));
    std::memcpy(buffer + sizeof(Magnum::Vector3), &obj.rotation, sizeof(Magnum::Quaternion));
}

inline void deserialize(Objet3D& obj, const char* buffer) {
    std::memcpy(&obj.position, buffer, sizeof(Magnum::Vector3));
    std::memcpy(&obj.rotation, buffer + sizeof(Magnum::Vector3), sizeof(Magnum::Quaternion));
}

// Network/Serialization.hpp
inline void deserializeEntityState(entt::registry& registry, entt::entity entity, const char* buffer) {
    auto& obj = registry.get<Objet3D>(entity);
    deserialize(obj, buffer);

    if(obj.rigidbody) {
        btTransform trans;
        trans.setOrigin(btVector3(obj.position.x(), obj.position.y(), obj.position.z()));
        trans.setRotation(btQuaternion(obj.rotation.vector().x(), obj.rotation.vector().y(), obj.rotation.vector().z(), obj.rotation.scalar()));
        obj.rigidbody->setWorldTransform(trans);
    }
}