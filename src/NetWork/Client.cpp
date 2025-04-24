#include "Client.hpp"
#include <enet/enet.h>
#include <iostream>
#include <cstring>

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
void Client::run( std::shared_ptr<Shared_Input> inputState, std::shared_ptr<Shared_Objects> objectState) {
    _running = true;
    _thread = std::thread(&Client::loop, this, inputState, objectState);
}


void Client::loop(std::shared_ptr<Shared_Input> inputState, std::shared_ptr<Shared_Objects> objectState) {
    ENetEvent event;
    while (_running) {
        std::cout << "Waiting for events...\n";
        std::flush(std::cout);

        // Check for input actions
        auto inputActions = inputState->getInputActions();
        // send every action to the server
        for ( auto action : inputActions) {
            // Serialize the action
            // create packet with a tableau of uint8_t
            // uint8_t data[sizeof(action)];
            // TODO : :D
            uint8_t type = 0;
            uint8_t data = static_cast<uint8_t>(action);
            //fusionne data et type
            uint8_t buffer[2];
            buffer[0] = type;
            buffer[1] = data;
            // Créer un paquet ENet
            ENetPacket* packet = enet_packet_create(&buffer, sizeof(buffer), ENET_PACKET_FLAG_RELIABLE);
            // Envoyer le paquet au serveur
            enet_peer_send(peer, 0, packet);
            std::cout << "Forward action\n";
        }
        inputState->clearInputActions();


        int result = enet_host_service(client, &event, 16);
        if (result > 0) {
            std::cout << "Event received\n";
            std::flush(std::cout);
            uint8_t* data;
            size_t dataSize;

            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "server connected.\n";
                std::flush(std::cout);
                break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "server disconnected.\n";
                std::flush(std::cout);
                break;
                case ENET_EVENT_TYPE_RECEIVE:
                    //get the data
                    data = event.packet->data;
                    dataSize = event.packet->dataLength;
                    if (data[0] == 1) { // its a object action
                        // Deserialize the action
                        Vector3 position;
                        Vector4 falseRotation;

                        // Deserialize position (3 floats)
                        std::memcpy(&position[0], &data[1], sizeof(float)); // x
                        std::memcpy(&position[1], &data[1 + sizeof(float)], sizeof(float)); // y
                        std::memcpy(&position[2], &data[1 + 2 * sizeof(float)], sizeof(float)); // z

                        // Deserialize rotation (3 floats)
                        std::memcpy(&falseRotation[2], &data[1 + 3 * sizeof(float)], sizeof(uint32_t)); // w

                        std::memcpy(&falseRotation[0], &data[1 + 3 * sizeof(float)+ 1 * sizeof(uint32_t) ], sizeof(uint32_t)); // x

                        std::memcpy(&falseRotation[3], &data[1 + 3 * sizeof(float)+ 2 * sizeof(uint32_t) ], sizeof(uint32_t)); // y

                        std::memcpy(&falseRotation[1], &data[1 + 3 * sizeof(float)+ 3 * sizeof(uint32_t) ], sizeof(uint32_t)); // z

                        Quaternion rotation{{falseRotation[3] , falseRotation[1], falseRotation[2]}, falseRotation[0] };

                        objectState->addCameraPosition(position, rotation);
                        std::cout << "Position : (" << position.x() << ", " << position.y() << ", " << position.z() << ")" << std::endl;
                        std::cout << "Rotation : (" << rotation.wxyz().w() << ", " << rotation.wxyz().x() << ", " << rotation.wxyz().y() << ", " << rotation.wxyz().z() << ")" << std::endl;
                        std::cout << "Received object action\n";
                    }
                    enet_packet_destroy(event.packet);
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
void Client::stop() {
    _running = false;
    if (peer)
        enet_peer_disconnect(peer, 0);
    if (client) {
        enet_host_destroy(client);
        client = nullptr;
    }
    enet_deinitialize();
    if (_thread.joinable())
        _thread.join();
}

void Client::disconnect() {
    _running = false;
    if (peer)
        enet_peer_disconnect(peer, 0);

    if (client)
        enet_host_destroy(client);

    enet_deinitialize();
    if (_thread.joinable())
        _thread.join();
}
