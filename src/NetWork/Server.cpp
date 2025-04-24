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
    _running = true;
    _thread = std::thread(&Server::loop, this);
}

void Server::stop() {
    _running = false;
    if (server) {
        enet_host_destroy(server);
        server = nullptr;
    }
    enet_deinitialize();
    if (_thread.joinable())
        _thread.join();
}

void Server::loop() {
    ENetEvent event;
    while (_running) {
        std::cout << "Waiting for events...\n";
        std::flush(std::cout);

        int result = enet_host_service(server, &event, 16);
        if (result > 0) {
            std::cout << "Event received\n";
            std::flush(std::cout);
            uint8_t* data;
            size_t dataSize;
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "Client connected.\n";
                std::flush(std::cout);
                break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Client disconnected.\n";
                std::flush(std::cout);
                break;
                case ENET_EVENT_TYPE_RECEIVE:
                    //get the data
                    data = event.packet->data;
                    dataSize = event.packet->dataLength;
                    if (data[0] == 0) // its a input action
                        std::cout << "Received input action: " << (int)data[1] << "\n";
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
