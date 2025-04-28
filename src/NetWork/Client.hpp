#pragma once
#include <memory>
#include <thread>
#include <enet/enet.h>

#include "Shared_Input.h"
#include "Shared_Objects.h"

class Client {
public:
    bool connectToServer(const char* host, enet_uint16 port);

    void run(std::shared_ptr<Shared_Input> inputState, std::shared_ptr<Shared_Objects> objectState);
    int getPlayerNumber() const { return playerNumber; }

    void stop();
    void disconnect();

private:
    void loop(std::shared_ptr<Shared_Input> inputState, std::shared_ptr<Shared_Objects> objectState);
    ENetHost* client = nullptr;
    ENetPeer* peer = nullptr;
    int playerNumber = -1;
    std::thread _thread;
    std::atomic<bool> _running{false};
};
