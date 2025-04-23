#pragma once
#include <entt/entt.hpp>
#include <Magnum/Math/Vector2.h>
#include "../ECS/Components.hpp"
#include "../Utils/Input.hpp"
#include "../Physics/PhysicsSystem.hpp"
#include "../Network/Client.hpp"

void UpdateInput(entt::registry& registry, Magnum::Vector2& input);
void UpdateNetwork(entt::registry& registry, Client& client);
void UpdatePhysics(entt::registry& registry, PhysicsSystem& physics, float dt);
