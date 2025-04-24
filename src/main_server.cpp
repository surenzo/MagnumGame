#include "Magnum/Timeline.h"
#include "Network/Server.hpp"
#include "physics/PhysicsSystem.hpp"

class ServerApplication {
public:
    ServerApplication();
    void loop();
    void tick();
private:
    Timeline _timeline;

    PhysicsSystem _physicSystem;
};

ServerApplication::ServerApplication() {
    auto* ground = _physicSystem.addBox({100.0f, 0.5f, 100.0f}, 0.0f);

    for(Int i = 0; i != 5; ++i) {
        for(Int j = 0; j != 5; ++j) {
            for(Int k = 0; k != 5; ++k) {
                auto* o = _physicSystem.addBox(Vector3{0.5f});
                o->translate({i - 2.0f, j + 4.0f, k - 2.0f});
                o->syncPose();
            }
        }
    }
    _timeline.start();
}

void ServerApplication::loop() {
    while (true) {
        tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // approx 60 FPS
    }
}

void ServerApplication::tick() {
    _physicSystem.update(_timeline.previousFrameDuration());
    // Here you can add code to send the state of the objects to the clients
    // For example, serialize the positions and rotations of the objects and send them over the network
    // You can use your serialization functions from Serialization.hpp
    _timeline.nextFrame();
}


int main() {
    Server server;
    ServerApplication app;
    if (!server.start(20000))
        return -1;

    server.run();
    app.loop();

    //server.stop();
    return 0;
}
