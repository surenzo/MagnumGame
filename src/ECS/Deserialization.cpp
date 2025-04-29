#pragma once
#include <cstdint>
#include <vector>
#include <cstring>
#include <iostream>
#include <entt/entt.hpp>

#include "Components.hpp"
// Décompression de quaternion
inline Quaternion readCompressedQuaternion(const uint8_t*& data) {
    uint8_t maxIndex = *data++;

    if (maxIndex >= 4 && maxIndex <= 7) {
        float x = (maxIndex == 4) ? 1.0f : 0.0f;
        float y = (maxIndex == 5) ? 1.0f : 0.0f;
        float z = (maxIndex == 6) ? 1.0f : 0.0f;
        float w = (maxIndex == 7) ? 1.0f : 0.0f;

        return Quaternion{{x, y, z,}, w};
    }

    int16_t a, b, c;
    std::memcpy(&a, data, sizeof(a));
    data += sizeof(a);
    std::memcpy(&b, data, sizeof(b));
    data += sizeof(b);
    std::memcpy(&c, data, sizeof(c));
    data += sizeof(c);

    const float FLOAT_PRECISION_MULT = 32767.0f;
    float aFloat = static_cast<float>(a) / FLOAT_PRECISION_MULT;
    float bFloat = static_cast<float>(b) / FLOAT_PRECISION_MULT;
    float cFloat = static_cast<float>(c) / FLOAT_PRECISION_MULT;
    float d = std::sqrt(1.0f - (aFloat * aFloat + bFloat * bFloat + cFloat * cFloat));

    if (maxIndex == 0)
        return Quaternion{{d, aFloat, bFloat}, cFloat};
    else if (maxIndex == 1)
        return Quaternion{{aFloat, d, bFloat}, cFloat};
    else if (maxIndex == 2)
        return Quaternion{{aFloat, bFloat, d}, cFloat};

    return Quaternion{{aFloat, bFloat, cFloat}, d};
}
inline void deserializeTransform(const uint8_t*& data, TransformComponent& transform) {
    uint32_t compressedPos;
    uint32_t compressedRot;

    // Extraire les données compressées
    std::memcpy(&compressedPos, data, sizeof(compressedPos));
    data += sizeof(compressedPos);
    // std::memcpy(&compressedRot, data + sizeof(compressedPos), sizeof(compressedRot));
    // data += sizeof(compressedRot);

    // Décompresser la position
    uint16_t x = (compressedPos >> 22) & 0x3FF;
    uint16_t y = (compressedPos >> 12) & 0x3FF;
    uint16_t z = compressedPos & 0x3FF;

    // Convertir les valeurs décompressées en float
    float posX = static_cast<float>(x) / 100.0f;
    float posY = static_cast<float>(y) / 100.0f;
    float posZ = static_cast<float>(z) / 100.0f;

    // Décompresser la rotation
    float rotX, rotY, rotZ, rotW;
    Quaternion quaternion = readCompressedQuaternion(data);

    // Reconstituer la TransformComponent
    transform.position = {posX, posY, posZ};
    transform.rotation = quaternion;
}

// void deserializeTransform(const uint8_t*& data, TransformComponent& transform) {
//     std::memcpy(&transform.position, data, sizeof(transform.position));
//     data += sizeof(transform.position);
//     std::memcpy(&transform.rotation, data, sizeof(transform.rotation));
//     data += sizeof(transform.rotation);
// }

inline void deserializeShape(const uint8_t*& data, ShapeComponent& shape) {
    // uint8_t type;
    // std::memcpy(&type, data, sizeof(type));
    // shape.type = static_cast<ShapeComponent::ShapeType>(type);
    // data += sizeof(type);
    //
    // std::memcpy(&shape.size, data, sizeof(shape.size));
    // data += sizeof(shape.size);
    //
    // std::memcpy(&shape.mass, data, sizeof(shape.mass));
    // data += sizeof(shape.mass);
    //
    // std::memcpy(&shape.radius, data, sizeof(shape.radius));
    // data += sizeof(shape.radius);


    uint8_t type = *data++;
    shape.type = static_cast<ShapeComponent::ShapeType>(type);

    uint8_t first = *data++;
    uint8_t second = *data++;
    uint8_t third = *data++;

    Vector3 size = {static_cast<float>(first), static_cast<float>(second), static_cast<float>(third)};
    shape.size = size;

    uint8_t mass = *data++;
    shape.mass = static_cast<float>(mass);

    uint8_t radius = *data++;
    shape.radius = static_cast<float>(radius);
}

inline void deserializeColor(const uint8_t*& data, RenderComponent& render) {
    // std::memcpy(&render.color, data, sizeof(render.color));
    // data += sizeof(render.color);
    // std::memcpy(&render.entityID, data, sizeof(render.entityID));
    // data += sizeof(render.entityID);

    // Extract RGB components
    uint8_t r = *data++;
    uint8_t g = *data++;
    uint8_t b = *data++;

    // Combine RGB into a single sRGB integer
    uint32_t SrgbInt = (static_cast<uint32_t>(r) << 16) |
                       (static_cast<uint32_t>(g) << 8) |
                       static_cast<uint32_t>(b);

    // Assign the reconstructed color
    render.color = Color3::fromSrgbInt(SrgbInt);

    // Extract and assign the entity ID
    uint16_t entityID;
    std::memcpy(&entityID, data, sizeof(uint16_t));
    render.entityID = static_cast<float>(entityID);
    data += sizeof(uint16_t);
}

inline void deserializeCamera(const uint8_t*& data, CameraComponent& camera) {
    // uint8_t id;
    // std::memcpy(&id, data, sizeof(uint8_t));
    // data += sizeof(uint8_t);
    // camera.id = static_cast<int>(id);
    camera.id = *data++;

    // std::memcpy(&camera.id, data, sizeof(uint8_t));
    // data += sizeof(uint8_t);
}

inline void deserializeRegistry(entt::registry& registry, const std::vector<uint8_t>& buffer) {
    const uint8_t* data = buffer.data();
    const uint8_t* end = data + buffer.size();

    while (data < end) {
        uint8_t type = *data;
        data += sizeof(uint8_t);


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
                TransformComponent transform;
                deserializeCamera(data, camera);
                deserializeTransform(data, transform);
                registry.emplace<CameraComponent>(entity, camera);
                registry.emplace<TransformComponent>(entity, transform);
                break;
            }
            default:
                std::cerr << "Unknown entity type: " << int(type) << "\n";
                return;
        }
    }
}