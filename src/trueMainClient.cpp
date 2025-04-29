#include "main.h"
#include "main_Client.h"

int main(int argc, char** argv) {
    std::string token;
    std::string address;
    Magnum::Platform::Application::Arguments arguments(argc, argv);
    auto inputStates = std::make_shared<Shared_Input>();
    auto ObjectStates = std::make_shared<Shared_Objects>();
    Client client;
    while(true){
        {
            Examples::ImGuiExample lobby(arguments);
            lobby.exec();
            token = lobby.getPlayerToken();
            address = lobby.getServerAddress();
            // parse the addresse to have addresse and port
        }

        std::string trueAdresseStr = address.substr(0, address.find(':'));
        const char* trueAdresse = trueAdresseStr.c_str();
        auto truePort = std::stoi(address.substr(address.find(':') + 1));

        if (!client.connectToServer(trueAdresse, truePort))
            return -1;
        client.setToken(token);

        // ici tu peux boucler pour interagir ou juste tester la connexion
        client.run(inputStates, ObjectStates);
        int playerNumber;
        while ((playerNumber = client.getPlayerNumber()) == -1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        {
            Game::MagnumBootstrap game(arguments, inputStates, ObjectStates);
            game.setPlayer(playerNumber);
            game.exec();
        }
      }
    return 0;
}
