#include <cstdint>
#include <vector>
#include <cstring>
#include <iostream>
#include <entt/entt.hpp>

#include "Components.hpp"

void deserializeTransform(const uint8_t*& data, TransformComponent& transform) {
    std::memcpy(&transform.position, data, sizeof(transform.position));
    data += sizeof(transform.position);
    std::memcpy(&transform.rotation, data, sizeof(transform.rotation));
    data += sizeof(transform.rotation);
}

void deserializeShape(const uint8_t*& data, ShapeComponent& shape) {
    uint8_t type;
    std::memcpy(&type, data, sizeof(type));
    shape.type = static_cast<ShapeComponent::ShapeType>(type);
    data += sizeof(type);

    std::memcpy(&shape.size, data, sizeof(shape.size));
    data += sizeof(shape.size);

    std::memcpy(&shape.radius, data, sizeof(shape.radius));
    data += sizeof(shape.radius);
}

void deserializeColor(const uint8_t*& data, RenderComponent& render) {
    std::memcpy(&render.color, data, sizeof(render.color));
    data += sizeof(render.color);
}

void deserializeCamera(const uint8_t*& data, CameraComponent& camera) {
    std::memcpy(&camera.id, data, sizeof(camera.id));
    data += sizeof(camera.id);
}

void deserializeRegistry(entt::registry& registry, const std::vector<uint8_t>& buffer) {
    const uint8_t* data = buffer.data();
    const uint8_t* end = data + buffer.size();

    while (data < end) {
        uint8_t type = *data;
        data += sizeof(uint8_t);

        uint32_t id;
        std::memcpy(&id, data, sizeof(id));
        data += sizeof(uint32_t);

        entt::entity entity = registry.create();

        switch (type) {
            case 0: { // Entity with Transform, Shape, Render
                TransformComponent transform;
                ShapeComponent shape;
                RenderComponent render;

                deserializeTransform(data, transform);
                deserializeShape(data, shape);
                deserializeColor(data, render);

                registry.emplace<TransformComponent>(entity, transform);
                registry.emplace<ShapeComponent>(entity, shape);
                registry.emplace<RenderComponent>(entity, render);
                break;
            }
            case 1: { // CameraComponent
                CameraComponent camera;
                deserializeCamera(data, camera);
                registry.emplace<CameraComponent>(entity, camera);
                break;
            }
            default:
                std::cerr << "Unknown entity type: " << int(type) << "\n";
                return;
        }
    }
}