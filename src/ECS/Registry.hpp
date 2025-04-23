#pragma once
#include <entt/entt.hpp>

class Registry {
public:
    static entt::registry& get() {
        static entt::registry instance;
        return instance;
    }
};