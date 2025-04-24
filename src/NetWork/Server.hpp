#pragma once
#include <thread>
#include <enet/enet.h>

class Server {
public:
    bool start(enet_uint16 port);
    void run();
    void stop();

private:
    ENetHost* server = nullptr;
    void loop();
    std::thread _thread;
    std::atomic<bool> _running{false};
};
