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
    tokens.resize(4);

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    server = enet_host_create(&address, 4, 1, 0, 0);
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

        if (!connectedClients.empty()) {
            //check for sending objects
            std::vector<uint8_t> buffer;
            uint8_t type =1;
            buffer.push_back(type);
            auto world = objectState->getWorld();
            buffer.insert(buffer.end(), world.begin(), world.end());
            // Create an ENet packet
            ENetPacket* packet = enet_packet_create(buffer.data(), buffer.size(), ENET_PACKET_FLAG_RELIABLE);

            // Send the packet to all clients
            for (auto& client : connectedClients) {
                enet_peer_send(client, 0, packet);
            }
        }

        int result = enet_host_service(server, &event, 16);
        if (result > 0) {
            std::flush(std::cout);
            uint8_t* data;
            size_t dataSize;
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "Client connected.\n";
                    connectedClients.push_back(event.peer);
                    // Send the player number to client
                    {
                        uint8_t buffer[2] = {3, static_cast<uint8_t>(connectedClients.size() - 1)}; // type 3 for welcome + player number
                        ENetPacket* packet = enet_packet_create(buffer, sizeof(buffer), ENET_PACKET_FLAG_RELIABLE);
                        enet_peer_send(event.peer, 0, packet);
                    }
                    if (!canStart && connectedClients.size() == 4) {
                        canStart = true;
                        std::cout << "Game started.\n";
                    }
                    std::flush(std::cout);
                break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Client disconnected.\n";
                    // remove the client from the list
                break;
                case ENET_EVENT_TYPE_RECEIVE:
                    //get the data
                    data = event.packet->data;
                    dataSize = event.packet->dataLength;
                    if (data[0] == 0) // its a input action
                    {
                        inputState->addInputAction(static_cast<InputAction>(data[2]), static_cast<int>(data[1]));
                    }
                    if (data[0] == 2) // input action with position
                    {
                        // Deserialize the action
                        InputAction action = static_cast<InputAction>(data[2]);
                        Magnum::Vector2 position;
                        std::memcpy(&position, data + 2, sizeof(Magnum::Vector2));
                        inputState->addClickAction(action, position, data[1]);
                    }
                    if (data[0] == 4) // its a token
                    {
                        std::string token(reinterpret_cast<char*>(data + 1), dataSize - 1);
                        std::cout << "Token received: " << token << "\n";
                        //find the number of the player and add its token
                        int playerNumber = -1;
                        for (size_t i = 0; i < connectedClients.size(); ++i) {
                            if (connectedClients[i] == event.peer) {
                                playerNumber = static_cast<int>(i);
                                break;
                            }
                        }
                        if (playerNumber != -1) {
                            tokens[playerNumber] = token;
                            std::cout << "Token for player " << playerNumber << ": " << token << "\n";
                        }
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
