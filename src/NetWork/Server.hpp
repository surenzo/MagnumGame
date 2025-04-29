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

private:
    ENetHost* server = nullptr;
    std::vector<ENetPeer*> connectedClients;
    std::vector<std::string> tokens;
    void loop(std::shared_ptr<Shared_Input> inputState, std::shared_ptr<Shared_Objects> objectState);
    bool canStart = false;
    std::thread _thread;
    std::atomic<bool> _running{false};
};
