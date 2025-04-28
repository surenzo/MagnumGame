#include <cstdint>
#include <vector>

#include "Components.hpp"
// Compression de quaternion
void writeCompressedQuaternion(std::vector<uint8_t>& out, const Quaternion& quaternion) {
    uint8_t maxIndex = 0;
    float maxValue = std::numeric_limits<float>::min();
    float sign = 1.0f;

    for (int i = 0; i < 4; ++i) {
        float element = quaternion.wxyz()[i];
        float abs = std::abs(element);
        if (abs > maxValue) {
            sign = (element < 0) ? -1.0f : 1.0f;
            maxIndex = static_cast<uint8_t>(i);
            maxValue = abs;
        }
    }

    if (std::abs(maxValue - 1.0f) < 0.0001f) {
        out.push_back(maxIndex + 4);
        return;
    }

    int16_t a, b, c;
    const float FLOAT_PRECISION_MULT = 32767.0f;

    if (maxIndex == 0) {
        a = static_cast<int16_t>(quaternion.wxyz()[1] * sign * FLOAT_PRECISION_MULT);
        b = static_cast<int16_t>(quaternion.wxyz()[2] * sign * FLOAT_PRECISION_MULT);
        c = static_cast<int16_t>(quaternion.wxyz()[3] * sign * FLOAT_PRECISION_MULT);
    } else if (maxIndex == 1) {
        a = static_cast<int16_t>(quaternion.wxyz()[0] * sign * FLOAT_PRECISION_MULT);
        b = static_cast<int16_t>(quaternion.wxyz()[2] * sign * FLOAT_PRECISION_MULT);
        c = static_cast<int16_t>(quaternion.wxyz()[3] * sign * FLOAT_PRECISION_MULT);
    } else if (maxIndex == 2) {
        a = static_cast<int16_t>(quaternion.wxyz()[0] * sign * FLOAT_PRECISION_MULT);
        b = static_cast<int16_t>(quaternion.wxyz()[1] * sign * FLOAT_PRECISION_MULT);
        c = static_cast<int16_t>(quaternion.wxyz()[3] * sign * FLOAT_PRECISION_MULT);
    } else {
        a = static_cast<int16_t>(quaternion.wxyz()[0] * sign * FLOAT_PRECISION_MULT);
        b = static_cast<int16_t>(quaternion.wxyz()[1] * sign * FLOAT_PRECISION_MULT);
        c = static_cast<int16_t>(quaternion.wxyz()[2] * sign * FLOAT_PRECISION_MULT);
    }

    out.push_back(maxIndex);
    out.insert(out.end(), reinterpret_cast<const uint8_t*>(&a), reinterpret_cast<const uint8_t*>(&a) + sizeof(a));
    out.insert(out.end(), reinterpret_cast<const uint8_t*>(&b), reinterpret_cast<const uint8_t*>(&b) + sizeof(b));
    out.insert(out.end(), reinterpret_cast<const uint8_t*>(&c), reinterpret_cast<const uint8_t*>(&c) + sizeof(c));
}

void serializeTransform(std::vector<uint8_t>& out, const TransformComponent& transform) {
    auto pos = transform.position;
    uint8_t sign = 0;
    sign |= (pos.x() < 0) ? 0x1 : 0;
    sign |= (pos.y() < 0) ? 0x2 : 0;
    sign |= (pos.z() < 0) ? 0x4 : 0;
    uint16_t x = static_cast<int>(abs(pos.x())*10) & 0x7FF;
    uint16_t y = static_cast<int>(abs(pos.y())*10) & 0x3FF;
    uint16_t z = static_cast<int>(abs(pos.z())*10) & 0x7FF;
    uint32_t compressedPos = (x << 21) | (y << 11) | z;


    auto rot = transform.rotation;
    out.push_back(sign);
    out.insert(out.end(), reinterpret_cast<const uint8_t*>(&compressedPos), reinterpret_cast<const uint8_t*>(&compressedPos) + sizeof(compressedPos));
    //out.push_back(compressedRot);

    // out.insert(out.end(), reinterpret_cast<uint8_t*>(&compressedPos), reinterpret_cast<uint8_t*>(&compressedPos) + sizeof(compressedPos));

    //out.insert(out.end(), reinterpret_cast<uint8_t*>(&compressedRot), reinterpret_cast<uint8_t*>(&compressedRot) + sizeof(compressedRot));

    //out.insert(out.end(), reinterpret_cast<uint8_t*>(&pos), reinterpret_cast<uint8_t*>(&pos) + sizeof(pos));
    //out.insert(out.end(), reinterpret_cast<uint8_t*>(&rot), reinterpret_cast<uint8_t*>(&rot) + sizeof(rot));
    writeCompressedQuaternion(out, rot);
}

void serializeShape(std::vector<uint8_t>& out, const ShapeComponent& shape) {
    out.push_back(static_cast<uint8_t>(shape.type));
    //
    // out.insert(out.end(), reinterpret_cast<const uint8_t*>(&shape.size), reinterpret_cast<const uint8_t*>(&shape.size) + sizeof(shape.size));
    // out.insert(out.end(), reinterpret_cast<const uint8_t*>(&shape.mass), reinterpret_cast<const uint8_t*>(&shape.mass) + sizeof(shape.mass));
    // out.insert(out.end(), reinterpret_cast<const uint8_t*>(&shape.radius), reinterpret_cast<const uint8_t*>(&shape.radius) + sizeof(shape.radius));

    // cast all of the data inside size into uint8_t
    uint8_t first = static_cast<uint8_t>(shape.size.x());
    uint8_t second = static_cast<uint8_t>(shape.size.y());
    uint8_t third = static_cast<uint8_t>(shape.size.z());

    out.push_back(first);
    out.push_back(second);
    out.push_back(third);

    out.push_back(static_cast<uint8_t>(shape.mass));
    out.push_back(static_cast<uint8_t>(shape.radius));
}

void serializeColor(std::vector<uint8_t>& out, const RenderComponent& render) {
    // auto color = render.color;
    // auto entityID = render.entityID;
    // out.insert(out.end(), reinterpret_cast<const uint8_t*>(&color), reinterpret_cast<const uint8_t*>(&color) + sizeof(color));
    // out.insert(out.end(), reinterpret_cast<const uint8_t*>(&entityID), reinterpret_cast<const uint8_t*>(&entityID) + sizeof(entityID));

    auto color = render.color;
    auto SrgbInt = color.toSrgbInt();
    // get the 24 first bits of r
    uint8_t r = (SrgbInt >> 16) & 0xFF;
    uint8_t g = (SrgbInt >> 8) & 0xFF;
    uint8_t b = SrgbInt & 0xFF;
    out.push_back(r);
    out.push_back(g);
    out.push_back(b);
    uint16_t entityID = render.entityID;
    out.insert(out.end(), reinterpret_cast<const uint8_t*>(&entityID), reinterpret_cast<const uint8_t*>(&entityID) + sizeof(uint16_t));
}
void serializeCamera(const CameraComponent& camera, std::vector<uint8_t>& out) {
    out.push_back(static_cast<uint8_t>(camera.id));

    //out.insert(out.end(), reinterpret_cast<const uint8_t>(&id), reinterpret_cast<const uint8_t>(&id) + sizeof(uint8_t));
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

        serializeTransform(buffer, transform);
        serializeShape(buffer, shape);
        serializeColor(buffer, render);
    }

    auto view2 = registry.view<TransformComponent,CameraComponent>();

    for (auto entity : view2) {
        const auto& transform = view2.get<TransformComponent>(entity);
        const auto& camera = view2.get<CameraComponent>(entity);

        uint8_t type = 1; // or some other identifier
        buffer.push_back(type);

        serializeCamera(camera, buffer);
        serializeTransform(buffer, transform);
    }

    return buffer;
}
