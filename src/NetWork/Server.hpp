#pragma once
#include <memory>
#include <thread>
#include <vector>
#include <enet/enet.h>

#include "Shared_Input.h"
#include "Shared_Objects.h"

class Server {
public:
    bool start(enet_uint16 port);
    void run(std::shared_ptr<Shared_Input> inputState, std::shared_ptr<Shared_Objects> objectState);
    void stop();
    bool canStartGame() const { return canStart; }
    void reset() {
        canStart = false;
        connectedClients.clear();
        tokens.clear();
    }
    std::string getWinnerToken(int player) {
        return tokens[player];
    }

    std::vector<std::string> getTokens() {
        return tokens;
    }

    void sendWinner(int player) {
        //check for sending objects
        std::vector<uint8_t> buffer;
        uint8_t type =6;
        buffer.push_back(type);
        buffer.push_back(static_cast<uint8_t>(player));
        // Create an ENet packet
        ENetPacket* packet = enet_packet_create(buffer.data(), buffer.size(), ENET_PACKET_FLAG_RELIABLE);
        // Send the packet to all clients
        for (auto& client : connectedClients) {
            enet_peer_send(client, 0, packet);
        }
    }

private:
    ENetHost* server = nullptr;
    std::vector<ENetPeer*> connectedClients;
    std::vector<std::string> tokens;
    void loop(std::shared_ptr<Shared_Input> inputState, std::shared_ptr<Shared_Objects> objectState);
    bool canStart = false;
    std::thread _thread;
    std::atomic<bool> _running{false};
};
