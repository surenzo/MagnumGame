#pragma once
#include <enet/enet.h>
#include <Magnum/Math/Vector2.h>
#include "../ECS/Components.hpp"
#include "../Network/Serialization.hpp"
#include <entt/entt.hpp>

class Client {
public:
    Client();
    ~Client();

    void connect(const char* host, uint16_t port);
    void sendInput(const Magnum::Vector2& input);
    void receive(entt::registry& registry);

private:
    ENetHost* client;
    ENetPeer* serverPeer;
};
