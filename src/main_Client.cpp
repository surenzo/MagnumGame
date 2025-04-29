#include "main_Client.h"

namespace Magnum::Game {

using namespace Math::Literals;

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

MagnumBootstrap::MagnumBootstrap(const Arguments& arguments, std::shared_ptr<Shared_Input> inputStates, std::shared_ptr<Shared_Objects> objectStates): Platform::Application(arguments, NoCreate){
    //config pas toucher --
    const Vector2 dpiScaling = this->dpiScaling({});
    Configuration conf;
    conf.setTitle("Magnum Bullet Integration Example")
        .setSize(conf.size(), dpiScaling);
    GLConfiguration glConf;
    glConf.setSampleCount(dpiScaling.max() < 2.0f ? 8 : 2);
    if(!tryCreate(conf, glConf))
        create(conf, glConf.setSampleCount(0));
    // -------
    inputState = inputStates;
    objectState = objectStates;
    _renderingSystem = std::make_unique<RenderingSystem>();
    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
        GL::Renderer::BlendEquation::Add);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
        GL::Renderer::BlendFunction::OneMinusSourceAlpha);

    _imgui = ImGuiIntegration::Context(Vector2{windowSize()}/this->dpiScaling(),
                windowSize(), framebufferSize());

    // probablement a changer
    (*(_cameraRig = new Object3D{&_scene}))
        .translate(Vector3::yAxis(3.0f));
    (*(_cameraObject = new Object3D{_cameraRig}))
        .translate(Vector3::zAxis(20.0f))
        .rotateX(-25.0_degf);
    (_camera = new SceneGraph::BasicCamera3D<float>(*_cameraObject))
        ->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.001f, 200.0f))
        .setViewport(GL::defaultFramebuffer.viewport().size());

    setSwapInterval(1);
    setMinimalLoopPeriod(16.0_msec);
    // TODO : changer le timeline pour la synchro
    _timeline.start();
}


    void MagnumBootstrap::viewportEvent(ViewportEvent& event) {
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});

    _imgui.relayout(Vector2{event.windowSize()}/event.dpiScaling(),
        event.windowSize(), event.framebufferSize());
}


    void MagnumBootstrap::keyReleaseEvent(KeyEvent& event) {
    if (_imgui.handleKeyReleaseEvent(event)) {
        event.setAccepted(); // Marque l'événement comme traité
        return;
    }
}


    void MagnumBootstrap::pointerReleaseEvent(PointerEvent& event) {
    if(_imgui.handlePointerReleaseEvent(event)) return;
}

    void MagnumBootstrap::pointerMoveEvent(PointerMoveEvent& event) {
    if(_imgui.handlePointerMoveEvent(event)) return;
}

void MagnumBootstrap::drawEvent() {
    if (_gameState == GAME_OVER) {
        std::cout << "Game Over" << std::endl;
        GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);

        _imgui.newFrame();

        ImGui::SetNextWindowSize(ImVec2(500, 100), ImGuiCond_FirstUseEver);
        ImGui::Begin("Fin de partie", &_showAnotherWindow);
        if (objectState->getWinner() == player) {
            ImGui::Text("Victoire");
            std::cout << "Victoire" << std::endl;
        } else {
            ImGui::Text("Defaite");
        }
        // add a button that just does exit()
        if (ImGui::Button("Exit", ImVec2(100, 30))) {
            exit();
        };
        ImGui::End();

        /* Update application cursor */
        _imgui.updateApplicationCursor(*this);

        /* Set appropriate states. If you only draw ImGui, it is sufficient to
           just enable blending and scissor test in the constructor. */
        GL::Renderer::enable(GL::Renderer::Feature::Blending);
        GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
        GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
        GL::Renderer::disable(GL::Renderer::Feature::DepthTest);

        _imgui.drawFrame();
        swapBuffers();
        redraw();
    }
    GL::Context::makeCurrent(&GL::Context::current());
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    auto packet = objectState->getWorld();
    auto winner = objectState->getWinner();
    if (winner!=-1) {
        _gameState = GAME_OVER;
    }

    entt::registry _newRegistry;
    deserializeRegistry(_newRegistry, packet);

    updateRegistry(_newRegistry);

    _renderingSystem.get()->render(_camera, _drawCubes, _drawDebug);

    swapBuffers();
    _timeline.nextFrame();
    redraw();
}

//     void printRegistery(const entt::registry& _registry) {
//     auto view = _registry.view<TransformComponent, ShapeComponent, RenderComponent>();
//     //TODO : fonctionne que sur windows
//     system("cls");
//     for (auto entity : view) {
//         const auto& transform = view.get<TransformComponent>(entity);
//         const auto& shape = view.get<ShapeComponent>(entity);
//         const auto& render = view.get<RenderComponent>(entity);
//
//         std::cout << "Entity: " << static_cast<unsigned int>(entity) << "\n";
//         std::cout << "  Transform: " << transform.position.x() << ", " << transform.position.y() << ", " << transform.position.z() << "\n";
//         std::cout << "  Rotation: " << transform.rotation.xyzw().x() << ", " << transform.rotation.xyzw().y() << ", " << transform.rotation.xyzw().z() << ", " << transform.rotation.xyzw().w() << "\n";
//         std::cout << "  Shape: " << (shape.type == ShapeComponent::ShapeType::Sphere ? "Sphere" : "Box") << "\n";
//     }
// }
    void printRegistery(const entt::registry& _registry){
    //clear std::cout
    //TODO : fonctionne que sur windows
    system("cls");
    auto view = _registry.view<TransformComponent, CameraComponent>();
    for (auto entity : view) {
        const auto& transform = view.get<TransformComponent>(entity);
        const auto& camera = view.get<CameraComponent>(entity);

        std::cout << "Entity: " << static_cast<unsigned int>(entity) << "\n";
        std::cout << "  Transform: " << transform.position.x() << ", " << transform.position.y() << ", " << transform.position.z() << "\n";
        std::cout << "  Rotation: " << transform.rotation.xyzw().x() << ", " << transform.rotation.xyzw().y() << ", " << transform.rotation.xyzw().z() << ", " << transform.rotation.xyzw().w() << "\n";
        std::cout << "  Camera ID: " << camera.id << "\n";
    }
}
void MagnumBootstrap::keyPressEvent(KeyEvent& event) {
    if (_gameState == GAME_OVER && _imgui.handleKeyPressEvent(event)) {
        event.setAccepted();
        return;
    }
    static const std::unordered_map<Key, std::function<void()>> keyActions{
    {Key::W, [this]() { inputState->addInputAction(InputAction::FORWARD); }},
    {Key::S, [this]() { inputState->addInputAction(InputAction::BACKWARD); }},
    {Key::A, [this]() { inputState->addInputAction(InputAction::LEFT); }},
    {Key::D, [this]() { inputState->addInputAction(InputAction::RIGHT); }},
    {Key::Q, [this]() { inputState->addInputAction(InputAction::UP); }},
    {Key::E, [this]() { inputState->addInputAction(InputAction::DOWN); }},
    {Key::Down, [this]() { inputState->addInputAction(InputAction::ROTATE_DOWN); }},
    {Key::Up, [this]() { inputState->addInputAction(InputAction::ROTATE_UP); }},
    {Key::Left, [this]() { inputState->addInputAction(InputAction::ROTATE_LEFT); }},
    {Key::Right, [this]() { inputState->addInputAction(InputAction::ROTATE_RIGHT); }},
        // if i press Y change the player to player 1
    {Key::Y, [this]() {player= (player+1)%4;}},
     {Key::B, [this]() { inputState->addInputAction(InputAction::B); printRegistery(_registry); }},
    {Key::C, [this]() {
            if (_drawCubes && _drawDebug) {
                _drawDebug = false;
            } else if (_drawCubes && !_drawDebug) {
                _drawCubes = false;
                _drawDebug = true;
            } else if (!_drawCubes && _drawDebug) {
                _drawCubes = true;
                _drawDebug = true;
            }
        }}
    };

    auto it = keyActions.find(event.key());
    if (it != keyActions.end()) {
        it->second(); // Execute the corresponding action
        event.setAccepted();
    }
    event.setAccepted();
}

void MagnumBootstrap::pointerPressEvent(PointerEvent& event) {
    if (_gameState == GAME_OVER && _imgui.handlePointerPressEvent(event)) return;
    if(!event.isPrimary() || !(event.pointer() & (Pointer::MouseLeft))) return;

    const Vector2 position = event.position() * Vector2{framebufferSize()} / Vector2{windowSize()};
    const Vector2 clickPoint = Vector2::yScale(-1.0f) * (position / Vector2{framebufferSize()} - Vector2{0.5f}) * _camera->projectionSize();

    inputState->addClickAction(InputAction::MOUSE_LEFT, clickPoint);
    event.setAccepted();
}

void MagnumBootstrap::updateRegistry(const entt::registry& newRegistry) {
    std::unordered_set<entt::entity> stillExists;

    auto view = newRegistry.view<TransformComponent, ShapeComponent, RenderComponent>();

    for (auto entity : view) {

        TransformComponent transform = view.get<TransformComponent>(entity);
        ShapeComponent shape = view.get<ShapeComponent>(entity);
        RenderComponent render = view.get<RenderComponent>(entity);

        float remoteID = render.entityID;

        if (linkingContext.contains(remoteID)) {
            entt::entity localEntity = linkingContext[remoteID];
            stillExists.insert(localEntity);

            // Update existing components
            auto& localTransform = _registry.get<TransformComponent>(localEntity);
            localTransform = transform;

            // Update SceneGraph object if needed
            if (auto* link = _registry.try_get<ObjectLinkComponent>(localEntity)) {
                link->object->setTransformation(Matrix4::translation(transform.position));
                link->object->rotateLocal(transform.rotation);
            }
            else
                std::cerr << "Object not found for entity " << static_cast<unsigned int>(localEntity) << std::endl;

        } else {
            // New entity
            entt::entity localEntity = _registry.create();
            linkingContext[remoteID] = localEntity;
            stillExists.insert(localEntity);

            _registry.emplace<TransformComponent>(localEntity, transform);
            _registry.emplace<ShapeComponent>(localEntity, shape);
            _registry.emplace<RenderComponent>(localEntity, render);

            auto* object = new Object3D{&_scene};
            switch (shape.type) {
                case ShapeComponent::ShapeType::Sphere:
                    _renderingSystem.get()->addSphere(*object, shape.radius, render.color);
                    break;
                case ShapeComponent::ShapeType::Box:
                    _renderingSystem.get()->addBox(*object, shape.size, render.color);
                    break;
            }
            object->setTransformation(Matrix4::translation(transform.position));
            object->rotateLocal(transform.rotation);

            _registry.emplace<ObjectLinkComponent>(localEntity, object);
        }
    }

    auto view2 = newRegistry.view<TransformComponent, CameraComponent>();

    for (auto entity : view2) {

        TransformComponent transform = view2.get<TransformComponent>(entity);
        CameraComponent camera = view2.get<CameraComponent>(entity);
        if (camera.id != player)
            continue;
        //std::cout << "Camera ID: " << camera.id << std::endl;
        _cameraObject->setTransformation(Matrix4::translation(transform.position));
        _cameraObject->rotateLocal(transform.rotation);
    }


    // Supprimer les entités locales qui ne sont plus dans le nouveau registre
    for (auto [remoteID, localEntity] : linkingContext) {
        if (!stillExists.contains(localEntity)) {
            if (auto* objLink = _registry.try_get<ObjectLinkComponent>(localEntity)) {
                delete objLink->object;
            }
            _registry.destroy(localEntity);
        }
    }

    // Nettoyage du linkingContext
    for (auto it = linkingContext.begin(); it != linkingContext.end(); ) {
        if (!stillExists.contains(it->second))
            it = linkingContext.erase(it);
        else
            ++it;
    }
}
}
// int  main(int argc, char** argv) {
//     auto inputStates = std::make_shared<Shared_Input>();
//     auto ObjectStates = std::make_shared<Shared_Objects>();
//     Client client;
//     if (!client.connectToServer("127.0.0.1", 20000))
//         return -1;
//
//     // ici tu peux boucler pour interagir ou juste tester la connexion
//     client.run(inputStates, ObjectStates);
//     int playerNumber;
//     while ((playerNumber = client.getPlayerNumber()) == -1) {
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
//     Magnum::Platform::Application::Arguments arguments(argc, argv);
//     Game::MagnumBootstrap app(arguments, inputStates, ObjectStates, playerNumber);
//     app.exec();
//     return 0;
// }


