#include <iostream>

#include "Network/Client.hpp"

int main() {
    Client client;
    if (!client.connectToServer("127.0.0.1", 20000))
        return -1;

    // ici tu peux boucler pour interagir ou juste tester la connexion
    std::cout << "Press Enter to quit...\n";
    std::cin.get();

    client.disconnect();
    return 0;
}
