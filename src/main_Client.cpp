#include <iostream>
#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/ImGuiIntegration/Context.hpp>
#include <Magnum/Math/Time.h>
#include <imgui.h>

#include "physics/PhysicsSystem.hpp"
#include "Rendering/RenderingSystem.h"
#include "Network/Client.hpp"
#include "NetWork/Shared_Input.h"
#include "NetWork/Shared_Objects.h"
#include "ECS/Deserialization.cpp"

namespace Magnum::Game {

using namespace Math::Literals;

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

class MagnumBootstrap: public Platform::Application {
public:
    explicit MagnumBootstrap(const Arguments& arguments, std::shared_ptr<Shared_Input> inputStates, std::shared_ptr<Shared_Objects> objectStates);

private:
    void drawEvent() override;
    void viewportEvent(ViewportEvent& event) override;
    void keyPressEvent(KeyEvent& event) override;
    void pointerPressEvent(PointerEvent& event) override;
    void updateRegistry(const entt::registry& newRegistry);
    Timeline _timeline;

    entt::registry _registry;
    std::unordered_map<uint32_t,entt::entity> linkingContext;

    PhysicsSystem _physicSystem;
    std::unique_ptr<RenderingSystem> _renderingSystem;

    Object3D *_cameraRig, *_cameraObject;
    SceneGraph::Camera3D* _camera;

    bool _drawCubes{true}, _drawDebug{true};
    ImGuiIntegration::Context _imgui{NoCreate};
    std::shared_ptr<Shared_Input> inputState;
    std::shared_ptr<Shared_Objects> objectState;
};

MagnumBootstrap::MagnumBootstrap(const Arguments& arguments, std::shared_ptr<Shared_Input> inputStates, std::shared_ptr<Shared_Objects> objectStates): Platform::Application(arguments, NoCreate) {
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



    // probablement a changer
    (*(_cameraRig = new Object3D{_physicSystem.getScene()}))
        .translate(Vector3::yAxis(3.0f))
        .rotateY(40.0_degf);
    (*(_cameraObject = new Object3D{_cameraRig}))
        .translate(Vector3::zAxis(20.0f))
        .rotateX(-25.0_degf);
    (_camera = new SceneGraph::BasicCamera3D<float>(*_cameraObject))
        ->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.001f, 100.0f))
        .setViewport(GL::defaultFramebuffer.viewport().size());
    //
    // auto* dummyBox = _physicSystem.addBox(Vector3{0.5f});
    // dummyBox->translate({0.0f, 0.0f, 0.0f});
    // dummyBox->syncPose();
    // _renderingSystem.get()->addBox(*dummyBox, Vector3{0.5f}, 0xffffff_rgbf);

    setSwapInterval(1);
    setMinimalLoopPeriod(16.0_msec);
    // TODO : changer le timeline pour la synchro
    _timeline.start();
}

void MagnumBootstrap::viewportEvent(ViewportEvent& event) {
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});
    _imgui.relayout(Vector2{event.windowSize()}/event.dpiScaling(), event.windowSize(), event.framebufferSize());
}

void MagnumBootstrap::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);
    // change the camera position based on what said the server:

    auto newRegistry = objectState->getWorld();

    // TODO : update the registry with the new objects
    entt::registry _newRegistry;
    deserializeRegistry(_newRegistry, newRegistry);

    updateRegistry(_newRegistry);

    if (_camera == nullptr) {
        std::cerr << "Camera object is null" << std::endl;
        return;
    }

    _physicSystem.getWorld()->stepSimulation( _timeline.previousFrameDuration(), 5);
    _renderingSystem.get()->render(_camera, _drawCubes, _drawDebug);

    swapBuffers();
    _timeline.nextFrame();
    redraw();
}

void MagnumBootstrap::keyPressEvent(KeyEvent& event) {
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
// if needed :
// { _cameraObject->translateLocal(Vector3::zAxis(-0.1f)); }}
// { _cameraObject->translateLocal(Vector3::zAxis(0.1f)); }},
// { _cameraObject->translateLocal(Vector3::xAxis(-0.1f)); }}
// { _cameraObject->translateLocal(Vector3::xAxis(0.1f)); }},
// { _cameraObject->translateLocal(Vector3::yAxis(0.1f)); }},
// { _cameraObject->translateLocal(Vector3::yAxis(-0.1f)); }}
//         () { _cameraObject->rotateX(5.0_degf); }},
//          { _cameraObject->rotateX(-5.0_degf); }},
//         () { _cameraRig->rotateY(-5.0_degf); }},
//         ]() { _cameraRig->rotateY(5.0_degf); }},

void MagnumBootstrap::pointerPressEvent(PointerEvent& event) {
    if(!event.isPrimary() || !(event.pointer() & (Pointer::MouseLeft))) return;

    const Vector2 position = event.position() * Vector2{framebufferSize()} / Vector2{windowSize()};
    const Vector2 clickPoint = Vector2::yScale(-1.0f) * (position / Vector2{framebufferSize()} - Vector2{0.5f}) * _camera->projectionSize();

    inputState->addClickAction(InputAction::MOUSE_LEFT, clickPoint);

    // const Vector3 direction = (_cameraObject->absoluteTransformation().rotationScaling() * Vector3{clickPoint, -1.0f}).normalized();
    //
    // auto* object = _physicSystem.addSphere(1,5);
    // object->translate(_cameraObject->absoluteTransformation().translation());
    // object->syncPose();
    // _renderingSystem.get()->addSphere(*object, 0.25f, 0x220000_rgbf);
    //
    // object->rigidBody().setLinearVelocity(btVector3{direction * 25.f});

    event.setAccepted();
}

void MagnumBootstrap::updateRegistry(const entt::registry& newRegistry) {
    std::unordered_set<entt::entity> stillExists;

    auto view = newRegistry.view<TransformComponent, ShapeComponent, RenderComponent>();

    for (auto entity : view) {
        uint32_t remoteID = static_cast<uint32_t>(entity);

        TransformComponent transform = view.get<TransformComponent>(entity);
        ShapeComponent shape = view.get<ShapeComponent>(entity);
        RenderComponent render = view.get<RenderComponent>(entity);

        if (linkingContext.contains(remoteID)) {
            entt::entity localEntity = linkingContext[remoteID];
            stillExists.insert(localEntity);

            // Update existing components
            auto& localTransform = _registry.get<TransformComponent>(localEntity);
            localTransform = transform;

            auto& localRender = _registry.get<RenderComponent>(localEntity);
            localRender = render;

            auto& localShape = _registry.get<ShapeComponent>(localEntity);
            // Update shape if needed

            // Update SceneGraph object if needed
            if (auto* link = _registry.try_get<ObjectLinkComponent>(localEntity)) {
                if (localShape.type != shape.type) {
                    std::cout << "Shape type changed for entity " << static_cast<unsigned int>(localEntity) << std::endl;
                    //enleve object link component et le remplace par un nouveau
                    auto object = _physicSystem.addSphere(shape.radius, shape.mass);
                    _renderingSystem.get()->addSphere(*object, shape.radius, render.color);
                    link->object = object;
                }
                link->object->resetTransformation()
                    .translate(transform.position)
                    .rotate(transform.rotation);
            }
            else
                std::cerr << "Object not found for entity " << static_cast<unsigned int>(localEntity) << std::endl;

            localShape = shape;

        } else {
            // New entity
            entt::entity localEntity = _registry.create();
            linkingContext[remoteID] = localEntity;
            stillExists.insert(localEntity);

            _registry.emplace<TransformComponent>(localEntity, transform);
            _registry.emplace<ShapeComponent>(localEntity, shape);
            _registry.emplace<RenderComponent>(localEntity, render);

            // Création d'un objet SceneGraph
            // auto* object = new SceneGraph::Object<SceneGraph::MatrixTransformation3D>{scene};
            // object->translate(transform.position)
            //       .rotate(transform.rotation);
            RigidBody* object;
            switch (shape.type) {
                case ShapeComponent::ShapeType::Sphere:
                    std::cout << "Creating sphere" << std::endl;
                    object = _physicSystem.addSphere(shape.radius, shape.mass);
                    _renderingSystem.get()->addSphere(*object, shape.radius, render.color);
                    break;
                case ShapeComponent::ShapeType::Box:
                    object = _physicSystem.addBox(shape.size, shape.mass);
                    _renderingSystem.get()->addBox(*object, shape.size, render.color);
                    break;
            }
            object->translate(transform.position);
            object->rotate(transform.rotation);
            object->syncPose();

            _registry.emplace<ObjectLinkComponent>(localEntity, object);
        }
    }

    auto view2 = newRegistry.view<TransformComponent, CameraComponent>();

    for (auto entity : view2) {
        uint32_t remoteID = static_cast<uint32_t>(entity);

        TransformComponent transform = view2.get<TransformComponent>(entity);
        CameraComponent camera = view2.get<CameraComponent>(entity);
        if (camera.id != 0)
            continue;


        //change the camera position
        _cameraObject->resetTransformation()
            .translate(transform.position)
            .rotate(transform.rotation);
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
int  main(int argc, char** argv) {
    auto inputStates = std::make_shared<Shared_Input>();
    auto ObjectStates = std::make_shared<Shared_Objects>();
    Client client;
    if (!client.connectToServer("127.0.0.1", 20000))
        return -1;

    // ici tu peux boucler pour interagir ou juste tester la connexion
    client.run(inputStates, ObjectStates);
    Magnum::Platform::Application::Arguments arguments(argc, argv);
    Game::MagnumBootstrap app(arguments, inputStates, ObjectStates);
    app.exec();
    return 0;
}


