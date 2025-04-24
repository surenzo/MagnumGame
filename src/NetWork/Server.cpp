#include "Server.hpp"
#include <enet/enet.h>
#include <iostream>
#include <vector>
#include <cstring>

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

void Server::run(std::shared_ptr<Shared_Input> inputState, std::shared_ptr<Shared_Objects> objectState) {
    _running = true;
    _thread = std::thread([this, inputState, objectState]() {
        this->loop(inputState, objectState);
    });
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

void Server::loop(std::shared_ptr<Shared_Input> inputState, std::shared_ptr<Shared_Objects> objectState) {
    ENetEvent event;
    while (_running) {
        std::cout << "Waiting for events...\n";
        std::flush(std::cout);

        if (!connectedClients.empty()) {
            //check for sending objects
            auto objectActions = objectState->getCameraPosition();

            // Serialize the action
            uint8_t type = 1; // Example type for object actions
            Vector3 position = objectActions.first; // Assuming getCameraPosition() returns a pair of position and rotation
            Quaternion rotation = objectActions.second;
            std::cout << "Position : (" << position.x() << ", " << position.y() << ", " << position.z() << ")" << std::endl;
            std::cout << "Rotation : (" << rotation.wxyz().w() << ", " << rotation.wxyz().x() << ", " << rotation.wxyz().y() << ", " << rotation.wxyz().z() << ")" << std::endl;

            // Serialize position and rotation into a buffer
            std::vector<uint8_t> buffer;
            buffer.push_back(type); // Add type

            // Serialize position (x, y, z)
            for (float coord : {position.x(), position.y(), position.z()}) {
                uint8_t* coordBytes = reinterpret_cast<uint8_t*>(&coord);
                buffer.insert(buffer.end(), coordBytes, coordBytes + sizeof(float));
            }

            // Serialize Quaternion
            for (float coord : {rotation.wxyz().w(), rotation.wxyz().x(), rotation.wxyz().y(), rotation.wxyz().z()}) {
                uint8_t* coordBytes = reinterpret_cast<uint8_t*>(&coord);
                buffer.insert(buffer.end(), coordBytes, coordBytes + sizeof(float));
            }

            // Create an ENet packet
            ENetPacket* packet = enet_packet_create(buffer.data(), buffer.size(), ENET_PACKET_FLAG_RELIABLE);

            // Send the packet to the client
            enet_peer_send(connectedClients[0], 0, packet);
            std::cout << "Forwarded object action\n";
        }

        int result = enet_host_service(server, &event, 16);
        if (result > 0) {
            std::cout << "Event received\n";
            std::flush(std::cout);
            uint8_t* data;
            size_t dataSize;
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "Client connected.\n";
                    connectedClients.push_back(event.peer);
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
                    {
                        inputState->addInputAction(static_cast<InputAction>(data[1]));
                        std::cout << "Received input action: " << static_cast<int>(data[1]) << "\n";
                    }
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
