#pragma once
#include <enet/enet.h>

class Server {
public:
    bool start(enet_uint16 port);
    void run();
    void stop();

private:
    ENetHost* server = nullptr;
};
