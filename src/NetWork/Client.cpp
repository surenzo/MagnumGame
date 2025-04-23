#include "Client.hpp"
#include <iostream>

Client::Client() {
    if (enet_initialize() != 0) {
        std::cerr << "Failed to initialize ENet\n";
        std::exit(1);
    }
    client = enet_host_create(nullptr, 1, 2, 0, 0);
}

Client::~Client() {
    if (serverPeer) enet_peer_disconnect(serverPeer, 0);
    enet_host_destroy(client);
    enet_deinitialize();
}

void Client::connect(const char* host, uint16_t port) {
    ENetAddress address;
    enet_address_set_host(&address, host);
    address.port = port;
    serverPeer = enet_host_connect(client, &address, 2, 0);
}

void Client::sendInput(const Magnum::Vector2& input) {
    char data[2] = {static_cast<char>(input.x()), static_cast<char>(input.y())};
    ENetPacket* packet = enet_packet_create(data, sizeof(data), ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(serverPeer, 0, packet);
}

void Client::receive(entt::registry& registry) {
    ENetEvent event;
    while (enet_host_service(client, &event, 0) > 0) {
        if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            //deserializeEntityState(event.packet->data, registry);
            enet_packet_destroy(event.packet);
        }
    }
}
