#include "Server.hpp"
#include <enet/enet.h>
#include <iostream>

bool Server::start(enet_uint16 port) {
    if (enet_initialize() != 0) {
        std::cerr << "ENet initialization failed\n";
        return false;
    }

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    server = enet_host_create(&address, 2, 2, 0, 0); // max 2 clients, 2 channels
    if (!server) {
        std::cerr << "Failed to create server host\n";
        return false;
    }

    std::cout << "Server started on port " << port << "\n";
    return true;
}

void Server::run() {
    ENetEvent event;
    while (true) {
        std::cout << "Waiting for events...\n";
        std::flush(std::cout);

        int result = enet_host_service(server, &event, 1000);
        if (result > 0) {
            std::cout << "Event received\n";
            std::flush(std::cout);

            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "Client connected.\n";
                std::flush(std::cout);
                break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Client disconnected.\n";
                std::flush(std::cout);
                break;
                default:
                    std::cout << "Unhandled event type: " << event.type << "\n";
                std::flush(std::cout);
                break;
            }
        } else if (result < 0) {
            std::cerr << "Error in enet_host_service\n";
            break;
        }
    }
}

void Server::stop() {
    if (server) {
        enet_host_destroy(server);
        server = nullptr;
    }
    enet_deinitialize();
}
