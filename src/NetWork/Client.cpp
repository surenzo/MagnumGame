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

        // Check for input actions
        auto inputActions = inputState->getInputActions();
        auto inputActionsWithPosition = inputState->getInputActionsWithPosition();
        inputState->clearInputActions();

        // send every action to the server
        for ( auto action : inputActions) {
            // Serialize the action
            // create packet with a tableau of uint8_t
            // uint8_t data[sizeof(action)];
            // TODO : :D
            uint8_t type = 0;
            uint8_t player = static_cast<uint8_t>(playerNumber);
            uint8_t data = static_cast<uint8_t>(action);
            //fusionne data et type
            uint8_t buffer[3];
            buffer[0] = type;
            buffer[1] = player;
            buffer[2] = data;
            // Créer un paquet ENet
            ENetPacket* packet = enet_packet_create(&buffer, sizeof(buffer), ENET_PACKET_FLAG_RELIABLE);
            // Envoyer le paquet au serveur
            enet_peer_send(peer, 0, packet);
        }
        for (auto action : inputActionsWithPosition) {
            // Serialize the action
            // create packet with a tableau of uint8_t
            // uint8_t data[sizeof(action)];
            // TODO : :D
            uint8_t type = 2; // possible d'optimiser en ajoutant ici le numero du joueur
            uint8_t player = static_cast<uint8_t>(playerNumber);
            uint8_t data = static_cast<uint8_t>(action.first);
            //fusionne data et type
            uint8_t buffer[3 + sizeof(action.second)];
            buffer[0] = type;
            buffer[1] = player;
            buffer[2] = data;
            memcpy(buffer + 3, &action.second, sizeof(action.second));
            // Créer un paquet ENet
            ENetPacket* packet = enet_packet_create(&buffer, sizeof(buffer), ENET_PACKET_FLAG_RELIABLE);
            // Envoyer le paquet au serveur
            enet_peer_send(peer, 0, packet);
        }

        int result = enet_host_service(client, &event, 16);
        if (result > 0) {
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
                        std::vector<uint8_t> buffer(data + 1, data + dataSize);
                        objectState->setWorld(buffer);
                    }
                    if (data[0] == 3) { // its a welcome message
                        playerNumber = data[1];
                        std::cout << "Welcome to the game! You are player number " << static_cast<int>(playerNumber) << "\n";
                    }
                    enet_packet_destroy(event.packet);
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
