#include "main_server.h"



ServerApplication::ServerApplication(std::shared_ptr<Shared_Input> inputStates, std::shared_ptr<Shared_Objects> objectStates){
    _registry = entt::registry();
    inputState = inputStates;
    objectState = objectStates;
    // add new entity to the registry

}
void ServerApplication::reset() {
    _registry.clear();
    _physicSystem.reset();
    _cameraRig.clear();
    _cameraObject.clear();

    entityID = 0;
    nbOfCubes.clear();

}

void ServerApplication::startGame() {
    nbOfCubes.resize(4);
    // Create 4 parent objects for the 4 players (platforms)
    for (int i = 0; i < 4; ++i) {
        auto cameraEntity = _registry.create();

        _cameraRig.emplace_back(new Object3D{_physicSystem.getScene()});
        _cameraRig[i]
            ->translateLocal(Vector3::yAxis(3.0f));

        _cameraObject.emplace_back(new Object3D{_cameraRig[i]});
        _cameraObject[i]
            ->translate(Vector3(40.0f * (i % 2), 8.0f, 40.0f * (i / 2)))
            .rotateXLocal(-25.0_degf);

        Matrix4 cameraMatrix = Matrix4::lookAt(
            _cameraObject[i]->transformationMatrix().translation(), // Position de la caméra
            Vector3{20.0f, 0.0f, 20.0f}, // Point que la caméra regarde
            Vector3{0.0f, 1.0f, 0.0f}  // Vecteur "up" (généralement l'axe Y)
        );
        std::cout << "Camera position : " << _cameraObject[i]->transformationMatrix().translation().x() << ", " << _cameraObject[i]->transformationMatrix().translation().y() << ", " << _cameraObject[i]->transformationMatrix().translation().z() << std::endl;

        // Ensuite, vous pouvez utiliser cette matrice pour orienter votre caméra
        _cameraObject[i]->setTransformation(cameraMatrix);

        Matrix4 cameraTransform = _cameraObject[i]->transformationMatrix();
        Vector3 position = cameraTransform.translation();
        Quaternion rotation = Quaternion::fromMatrix(cameraTransform.rotation());


        _registry.emplace<TransformComponent>(cameraEntity, position, rotation);
        _registry.emplace<CameraComponent>(cameraEntity, /*id=*/i);
        _registry.emplace<CameraLinkComponent>(cameraEntity, _cameraObject[i]);

        auto ground = createAndAddEntity(-1,_physicSystem.getScene(), ShapeComponent::ShapeType::Box, {15.0f, 1.0f, 15.0f}, 0.0f, 0x220000_rgbf);
        ground->translate(Vector3(40.0f * (i % 2), -1.0f, 40.0f * (i / 2)));
        ground->syncPose();
        Deg hue = 42.0_degf;
        // Stacked boxes
        for (Int i = 0; i != 5; ++i) {
            for (Int j = 0; j != 5; ++j) {
                for (Int k = 0; k != 5; ++k) {
                    auto box = createAndAddEntity(
                        i,
                        _physicSystem.getScene(),
                        ShapeComponent::ShapeType::Box,
                        Vector3{1.0f},
                        1.0f,
                        Color3::fromHsv({hue , 0.75f, 0.9f}));
                    box->translate({i - 2.0f, j + 3.0f, k - 2.0f});
                    box->translate(Vector3(40.0f * (i % 2), 0.0f, 40.0f * (i / 2)));

                    box->syncPose();
                }
            }
        }
    }

    actionHandlers = std::unordered_map<InputAction, std::function<void(int)>> {
                {InputAction::FORWARD, [this](int player) { _cameraObject[player]->translateLocal(Vector3::zAxis(-.5f)); }},
                {InputAction::BACKWARD, [this](int player) { _cameraObject[player]->translateLocal(Vector3::zAxis(.5f)); }},
                {InputAction::LEFT, [this](int player) { _cameraObject[player]->translateLocal(Vector3::xAxis(-.5f)); }},
                {InputAction::RIGHT, [this](int player) { _cameraObject[player]->translateLocal(Vector3::xAxis(.5f)); }},
                {InputAction::UP, [this](int player) { _cameraObject[player]->translateLocal(Vector3::yAxis(.5f)); }},
                {InputAction::DOWN, [this](int player) { _cameraObject[player]->translateLocal(Vector3::yAxis(-.5f)); }},
                {InputAction::ROTATE_UP, [this](int player) { _cameraObject[player]->rotateXLocal(-5.0_degf); }},
                {InputAction::ROTATE_DOWN, [this](int player) { _cameraObject[player]->rotateXLocal(5.0_degf); }},
                {InputAction::ROTATE_LEFT, [this](int player) { _cameraObject[player]->rotateY(-5.0_degf); }},
                {InputAction::ROTATE_RIGHT, [this](int player) { _cameraObject[player]->rotateY(5.0_degf); }},

            };
    _timeline.start();
}

void ServerApplication::updateRegistry() {
    // Sync des objets physiques (rigidbodies)
    auto physicsView = _registry.view<TransformComponent, PhysicsLinkComponent>();
    for (auto entity : physicsView) {
        auto& transform = physicsView.get<TransformComponent>(entity);
        auto* body = physicsView.get<PhysicsLinkComponent>(entity).body;

        const auto& matrix = body->transformationMatrix();
        transform.position = matrix.translation();
        transform.rotation = Magnum::Quaternion::fromMatrix(matrix.rotation());
    }

    // Sync des caméras attachées à des objets 3D
    auto cameraView = _registry.view<TransformComponent, CameraLinkComponent>();
    for (auto entity : cameraView) {
        auto& transform = cameraView.get<TransformComponent>(entity);
        auto* cameraObject = cameraView.get<CameraLinkComponent>(entity).cameraObject;

        const auto& matrix = cameraObject->transformation();
        transform.position = matrix.translation();
        transform.rotation = Magnum::Quaternion::fromMatrix(matrix.rotation());
    }
}

std::tuple<int, std::vector<int>>  ServerApplication::loop() {
    bool running = true;
    int winner = -1;
    startGame();
    while (running) {
        tick();
        for ( int i = 0; i < nbOfCubes.size(); ++i) {
            if (nbOfCubes[i] == 0) {
                std::cout << "Player " << i << " wins!" << std::endl;
                winner = i;
                running = false;
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Sleep for 3ms to limit CPU usage
    }
    return {winner, nbOfCubes};
}

void ServerApplication::tick() {

    PlayInputs();
    updateRegistry();
    auto packet = serializeRegistry(_registry);
    objectState->setWorld(packet);


    std::vector<Object3D*> entitesToDestroy = _physicSystem.update(_timeline.previousFrameDuration());

    for (auto obj : entitesToDestroy) {
        // remove the object from the scene
        delete obj;
        // remove the object from the registry
        auto view = _registry.view<PhysicsLinkComponent,PlayerLinkComponent>();
        for (auto entity : view) {
            if (view.get<PhysicsLinkComponent>(entity).body == obj) {
                nbOfCubes[view.get<PlayerLinkComponent>(entity).id]--;
                _registry.destroy(entity);
                break;
            }
        }
    }


    _timeline.nextFrame();
}
void ServerApplication::PlayInputs() {
    //play inputs
    auto inputActions = inputState->getInputActionsWithPlayer();
    auto inputActionsWithPosition = inputState->getInputActionsWithPositionWithPlayer();
    inputState->clearInputActionsWithPlayer();

    //std::cout << "Nombres d'Input actions: " << inputActions.size() << " ," << inputActionsWithPosition.size() << std::endl;
    for (auto action : inputActions) {
        auto it = actionHandlers.find(action.first);
        if (it != actionHandlers.end()) {
            it->second(action.second); // Execute the corresponding action
        }
        else
            std::cout << "Action not found: " << static_cast<int>(action.first) << std::endl;
    }

    //do this if there is a click :

    for (auto action : inputActionsWithPosition) {
        if (std::get<0>(action) == InputAction::MOUSE_LEFT) {
            const Vector2 position = std::get<1>(action);
            const int player = std::get<2>(action);
            const Vector3 direction = (_cameraObject[player]->absoluteTransformation().rotationScaling() * Vector3{position, -1.0f}).normalized();

            auto* sphere = createAndAddEntity(-1,_physicSystem.getScene(),ShapeComponent::ShapeType::Sphere, Vector3{1.0f}, 5.0f, 0x220000_rgbf);

            sphere->translate(_cameraObject[player]->absoluteTransformation().translation());
            sphere->syncPose();
            sphere->rigidBody().setLinearVelocity(btVector3{direction * 25.f});
        }
    }
}



RigidBody* ServerApplication::createAndAddEntity(
    int player,
    Object3D* parent,
    ShapeComponent::ShapeType shapeType,
    const Vector3& sizeOrRadius,
    float mass,
    const Color3& color
) {
    RigidBody* body = nullptr;

    // Create the physics object based on the shape type
    if (shapeType == ShapeComponent::ShapeType::Sphere) {
        body = _physicSystem.addSphere(parent, sizeOrRadius.x(), mass);
    } else if (shapeType == ShapeComponent::ShapeType::Box) {
        body = _physicSystem.addBox(parent, sizeOrRadius, mass);
    }

    if (!body) {
        std::cerr << "Failed to create physics body for entity!" << std::endl;
        return nullptr;
    }

    // Create the entity in the registry
    auto entity = _registry.create();

    // Add components to the entity
    _registry.emplace<TransformComponent>(
        entity,
        body->transformationMatrix().translation(),
        Quaternion::fromMatrix(body->transformationMatrix().rotation())
    );

    _registry.emplace<ShapeComponent>(
        entity,
        shapeType,
        mass,
        shapeType == ShapeComponent::ShapeType::Box ? sizeOrRadius : Vector3{0.0f}, // size for boxes
        shapeType == ShapeComponent::ShapeType::Sphere ? sizeOrRadius.x() : 0.0f    // radius for spheres
    );

    _registry.emplace<RenderComponent>(
        entity,
        color,
        entityID++ // ID of the entity for rendering
    );

    _registry.emplace<PhysicsLinkComponent>(
        entity,
        body
    );
    if (player != -1) {
        _registry.emplace<PlayerLinkComponent>(
            entity,
            player
        );
        nbOfCubes[player]++;
    }

    return body;
}


int main(int argc, char** argv) {

    //verifie que argc est bien 2
    //et les deux premiers parametres sont l'addresse et le port
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <address:port>" << std::endl;
        return -1;
    }
    //parse the addresse and port
    std::string address = argv[0];
    std::string port = argv[1];
    std::string serverIp = address + ":" + port;
    std::shared_ptr<Shared_Input> inputStates = std::make_shared<Shared_Input>();
    std::shared_ptr<Shared_Objects> objectStates = std::make_shared<Shared_Objects>();
    Server server;

    ServerApplication app{inputStates, objectStates};
    if (!server.start(20000))
        return -1;

    while (true) {
        //send the fact that the server is up to go
        // requete post pour l'addresse sur l'api
        server.run(inputStates, objectStates);
        httplib::Client client("http://192.168.87.47:5160");
        nlohmann::json body = {{"ServerIp", serverIp}};
        std::cerr << "Nouveau serveur: " << serverIp << " <address:port>" << std::endl;

        auto response = client.Post("/api/Matchmaking/register", body.dump(), "application/json");
        while (!server.canStartGame()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        //send that the server is running
        auto [winner,cubes] = app.loop();
        server.sendWinner(winner);
        auto token = server.getWinnerToken(winner);
        //send the token / winner to the server API

        server.reset();
        app.reset();
    }

    //server.stop();
    return 0;
}
