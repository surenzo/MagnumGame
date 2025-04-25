#include <cstdint>
#include <vector>

#include "Components.hpp"

void serializeTransform(std::vector<uint8_t>& out, const TransformComponent& transform) {
    auto pos = transform.position;
    auto rot = transform.rotation;

    out.insert(out.end(), reinterpret_cast<uint8_t*>(&pos), reinterpret_cast<uint8_t*>(&pos) + sizeof(pos));
    out.insert(out.end(), reinterpret_cast<uint8_t*>(&rot), reinterpret_cast<uint8_t*>(&rot) + sizeof(rot));
}

void serializeShape(std::vector<uint8_t>& out, const ShapeComponent& shape) {
    auto type = static_cast<uint8_t>(shape.type);
    out.push_back(type);

    out.insert(out.end(), reinterpret_cast<const uint8_t*>(&shape.size), reinterpret_cast<const uint8_t*>(&shape.size) + sizeof(shape.size));
    out.insert(out.end(), reinterpret_cast<const uint8_t*>(&shape.radius), reinterpret_cast<const uint8_t*>(&shape.radius) + sizeof(shape.radius));
}

void serializeColor(std::vector<uint8_t>& out, const RenderComponent& render) {
    auto color = render.color;
    out.insert(out.end(), reinterpret_cast<const uint8_t*>(&color), reinterpret_cast<const uint8_t*>(&color) + sizeof(color));
}
void serializeCamera(const CameraComponent& camera, std::vector<uint8_t>& out) {
    auto id = camera.id;
    out.insert(out.end(), reinterpret_cast<const uint8_t*>(&id), reinterpret_cast<const uint8_t*>(&id) + sizeof(id));
}

std::vector<uint8_t> serializeRegistry(const entt::registry& registry) {
    std::vector<uint8_t> buffer;

    auto view = registry.view<TransformComponent, ShapeComponent, RenderComponent>();

    for (auto entity : view) {
        const auto& transform = view.get<TransformComponent>(entity);
        const auto& shape = view.get<ShapeComponent>(entity);
        const auto& render = view.get<RenderComponent>(entity);
        // add type of entity
        uint8_t type = 0; // or some other identifier
        buffer.push_back(type);

        uint32_t id = static_cast<uint32_t>(entity); // or a custom ID system
        buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&id), reinterpret_cast<uint8_t*>(&id) + sizeof(uint32_t));


        serializeTransform(buffer, transform);
        serializeShape(buffer, shape);
        serializeColor(buffer, render);
    }

    auto view2 = registry.view<CameraComponent>();

    for (auto entity : view2) {
        const auto& camera = view2.get<CameraComponent>(entity);

        uint8_t type = 1; // or some other identifier
        buffer.push_back(type);

        uint32_t id = static_cast<uint32_t>(entity); // or a custom ID system
        buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&id), reinterpret_cast<uint8_t*>(&id) + sizeof(uint32_t));

        serializeCamera(camera, buffer);
    }

    return buffer;
}
