#include "Network/Server.hpp"

int main() {
    Server server;
    if (!server.start(20000))
        return -1;

    server.run();
    server.stop();
    return 0;
}
