#pragma once
#include <enet/enet.h>

class Client {
public:
    bool connectToServer(const char* host, enet_uint16 port);
    void disconnect();

private:
    ENetHost* client = nullptr;
    ENetPeer* peer = nullptr;
};
