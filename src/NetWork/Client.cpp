#include "Client.hpp"
#include <enet/enet.h>
#include <iostream>

bool Client::connectToServer(const char* host, enet_uint16 port) {
    if (enet_initialize() != 0) {
        std::cerr << "ENet initialization failed\n";
        return false;
    }

    client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (!client) {
        std::cerr << "Failed to create client host\n";
        return false;
    }

    ENetAddress address;
    enet_address_set_host(&address, host);
    address.port = port;

    peer = enet_host_connect(client, &address, 2, 0);
    if (!peer) {
        std::cerr << "Connection to " << host << " failed\n";
        return false;
    }

    std::cout << "Connecting to " << host << "...\n";

    ENetEvent event;
    if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        std::cout << "Connected to server\n";
        enet_host_flush(client);
        return true;
    }

    std::cerr << "Failed to connect\n";
    enet_peer_reset(peer);
    return false;
}

void Client::disconnect() {
    if (peer)
        enet_peer_disconnect(peer, 0);

    if (client)
        enet_host_destroy(client);

    enet_deinitialize();
}
