#include <iostream>

#include <Magnum/Platform/GlfwApplication.h>
#include "Magnum/Timeline.h"
#include <Magnum/Math/Time.h>
#include "Network/Server.hpp"
#include "Network/Shared_Input.h"
#include "Network/Shared_Objects.h"
#include "physics/PhysicsSystem.hpp"
#include "ECS/Serialization.cpp"
#include "Magnum/GL/DefaultFramebuffer.h"
#include "Magnum/SceneGraph/Camera.h"
#include "Rendering/RenderingSystem.h"

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
using namespace Math::Literals;
class ServerApplication : public Platform::Application {
public:
    explicit ServerApplication(const Arguments& arguments, std::shared_ptr<Shared_Input> inputStates, std::shared_ptr<Shared_Objects> objectStates);
    void updateRegistry();
    void loop();
    void tick();
private:
    void drawEvent() override;
    Timeline _timeline;
    entt::registry _registry;

    std::unique_ptr<RenderingSystem> _renderingSystem;
    PhysicsSystem _physicSystem;
    Object3D *_cameraRig, *_cameraObject;
    SceneGraph::Camera3D* _camera;

    float entityID = 0;
    std::shared_ptr<Shared_Input> inputState = std::make_shared<Shared_Input>();
    std::shared_ptr<Shared_Objects> objectState = std::make_shared<Shared_Objects>();
};

ServerApplication::ServerApplication(const Arguments& arguments, std::shared_ptr<Shared_Input> inputStates, std::shared_ptr<Shared_Objects> objectStates) :  Platform::Application(arguments, NoCreate){
    //config pas toucher --
    const Vector2 dpiScaling = this->dpiScaling({});
    Configuration conf;
    conf.setTitle("Magnum Bullet Integration Example")
        .setSize(conf.size(), dpiScaling);
    GLConfiguration glConf;
    glConf.setSampleCount(dpiScaling.max() < 2.0f ? 8 : 2);
    if(!tryCreate(conf, glConf))
        create(conf, glConf.setSampleCount(0));

    _registry = entt::registry();
    inputState = inputStates;
    objectState = objectStates;
    _renderingSystem = std::make_unique<RenderingSystem>();
    // add new entity to the registry

    auto cameraEntity = _registry.create();
    // add a transform component to the entity
    // add a camera component to the entity

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

    // Récupère la transformation globale (translation + rotation)
    Matrix4 globalTransform = _cameraObject->transformationMatrix();
    Vector3 position = globalTransform.translation();
    Quaternion rotation = Quaternion::fromMatrix(globalTransform.rotation());

    // Ajoute les composants
    _registry.emplace<TransformComponent>(cameraEntity, position, rotation);
    _registry.emplace<CameraComponent>(cameraEntity, /*id=*/0); // par exemple joueur 0
    _registry.emplace<CameraLinkComponent>(cameraEntity, _cameraObject);

    // 1. Sol
    {
        auto* ground = _physicSystem.addBox({20.0f, 0.5f, 20.0f}, 0.0f);
        _renderingSystem->addBox(*ground, {20.0f, 0.5f, 20.0f}, 0x220000_rgbf);

        auto groundEntity = _registry.create();

        _registry.emplace<TransformComponent>(
            groundEntity,
            ground->transformationMatrix().translation(),
            Quaternion::fromMatrix(ground->transformationMatrix().rotation())
        );

        _registry.emplace<ShapeComponent>(
            groundEntity,
            ShapeComponent::ShapeType::Box,
            0.0f, // masse
            Vector3{20.0f, 0.5f, 20.0f},
            0.0f
        );

        _registry.emplace<RenderComponent>(
            groundEntity,
            Color3{0.2f, 0.2f, 0.2f},
            entityID++ // ID de l'entité pour le rendu
        );

        _registry.emplace<PhysicsLinkComponent>(
            groundEntity,
            ground
        );
    }
    {
        auto* ground = _physicSystem.addSphere(10.f, 0.0f);
        _renderingSystem->addSphere(*ground, 10.f, 0x220000_rgbf);
        ground->getRigidBody().setWorldTransform(btTransform::getIdentity());

        auto groundEntity = _registry.create();

        _registry.emplace<TransformComponent>(
            groundEntity,
            ground->transformationMatrix().translation(),
            Quaternion::fromMatrix(ground->transformationMatrix().rotation())
        );

        _registry.emplace<ShapeComponent>(
            groundEntity,
            ShapeComponent::ShapeType::Sphere,
            0.0f, // masse
            Vector3{20.0f, 0.5f, 20.0f},
            10.f
        );

        _registry.emplace<RenderComponent>(
            groundEntity,
            Color3{0.2f, 0.2f, 0.2f},
            entityID++ // ID de l'entité pour le rendu
        );

        _registry.emplace<PhysicsLinkComponent>(
            groundEntity,
            ground
        );


    }

    Deg hue = 42.0_degf;
    // 2. Boîtes empilées
    for(Int i = 0; i != 5; ++i) {
        for(Int j = 0; j != 5; ++j) {
            for(Int k = 0; k != 5; ++k) {
                auto* box = _physicSystem.addBox(Vector3{0.5f});
                _renderingSystem->addBox(*box, Vector3{0.5f}, Color3::fromHsv({hue += 137.5_degf, 0.75f, 0.9f}));
                box->translate({i - 2.0f, j + 4.0f, k - 2.0f});
                box->syncPose();

                auto boxEntity = _registry.create();

                _registry.emplace<TransformComponent>(
                    boxEntity,
                    box->transformationMatrix().translation(),
                    Quaternion::fromMatrix(box->transformationMatrix().rotation())
                );

                _registry.emplace<ShapeComponent>(
                    boxEntity,
                    ShapeComponent::ShapeType::Box,
                    1.0f, // masse
                    Vector3{0.5f}, // taille
                    0.0f           // radius ignoré
                );

                _registry.emplace<RenderComponent>(
                    boxEntity,
                    Color3::fromHsv({hue += 137.5_degf, 0.75f, 0.9f}),
                    entityID++ // ID de l'entité pour le rendu
                );

                _registry.emplace<PhysicsLinkComponent>(
                    boxEntity,
                    box
                );
            }
        }
    }
    setSwapInterval(1);
    setMinimalLoopPeriod(16.0_msec);
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


void ServerApplication::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);
    tick();
    _renderingSystem.get()->render(_camera, true, true);

    swapBuffers();
    redraw();
}

void ServerApplication::loop() {
    while (true) {
        tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // approx 60 FPS
    }
}

void printRegistery(const entt::registry& _registry) {
    //clear std::cout
    //TODO : fonctionne que sur windows
    system("cls");
    auto view = _registry.view<TransformComponent, ShapeComponent, RenderComponent>();
    for (auto entity : view) {
        const auto& transform = view.get<TransformComponent>(entity);
        const auto& shape = view.get<ShapeComponent>(entity);
        const auto& render = view.get<RenderComponent>(entity);

        std::cout << "Entity: " << static_cast<unsigned int>(entity) << "\n";
        std::cout << "  Transform: " << transform.position.x() << ", " << transform.position.y() << ", " << transform.position.z() << "\n";
        std::cout << "  Rotation: " << transform.rotation.xyzw().x() << ", " << transform.rotation.xyzw().y() << ", " << transform.rotation.xyzw().z() << ", " << transform.rotation.xyzw().w() << "\n";
        std::cout << "  Shape: " << (shape.type == ShapeComponent::ShapeType::Sphere ? "Sphere" : "Box") << "\n";
    }
}
void ServerApplication::tick() {
    //play inputs
    auto inputActions = inputState->getInputActions();
    auto inputActionsWithPosition = inputState->getInputActionsWithPosition();
    inputState->clearInputActions();

    if (_cameraObject == nullptr) {
        std::cerr << "Camera object is null" << std::endl;
        return;
    }
    std::unordered_map<InputAction, std::function<void()>> actionHandlers{
        {InputAction::FORWARD, [this]() { _cameraObject->translateLocal(Vector3::zAxis(-0.1f)); }},
        {InputAction::BACKWARD, [this]() { _cameraObject->translateLocal(Vector3::zAxis(0.1f)); }},
        {InputAction::LEFT, [this]() { _cameraObject->translateLocal(Vector3::xAxis(-0.1f)); }},
        {InputAction::RIGHT, [this]() { _cameraObject->translateLocal(Vector3::xAxis(0.1f)); }},
        {InputAction::UP, [this]() { _cameraObject->translateLocal(Vector3::yAxis(0.1f)); }},
        {InputAction::DOWN, [this]() { _cameraObject->translateLocal(Vector3::yAxis(-0.1f)); }},
        {InputAction::ROTATE_UP, [this]() { _cameraObject->rotateX(-5.0_degf); }},
        {InputAction::ROTATE_DOWN, [this]() { _cameraObject->rotateX(5.0_degf); }},
        {InputAction::ROTATE_LEFT, [this]() { _cameraObject->rotateY(-5.0_degf); }},
        {InputAction::ROTATE_RIGHT, [this]() { _cameraObject->rotateY(5.0_degf); }},
           {InputAction::B, [this]() { printRegistery(_registry); }},
    };


    for (auto action : inputActions) {
        auto it = actionHandlers.find(action);
        if (it != actionHandlers.end()) {
            it->second(); // Execute the corresponding action
        }
    }

    //do this if there is a click :

    for (auto action : inputActionsWithPosition) {
        if (action.first == InputAction::MOUSE_LEFT) {
            const Vector2 position = action.second;
            const Vector3 direction = (_cameraObject->absoluteTransformation().rotationScaling() * Vector3{position, -1.0f}).normalized();

            auto* sphere = _physicSystem.addSphere(1, 5);
            _renderingSystem->addSphere(*sphere, 1.0f, 0x220000_rgbf);
            sphere->translate(_cameraObject->absoluteTransformation().translation());
            sphere->syncPose();

            auto sphereEntity = _registry.create();

            _registry.emplace<TransformComponent>(
                sphereEntity,
                sphere->transformationMatrix().translation(),
                Quaternion::fromMatrix(sphere->transformationMatrix().rotation())
            );

            _registry.emplace<ShapeComponent>(
                sphereEntity,
                ShapeComponent::ShapeType::Sphere,
                1.0f, // masse
                Vector3{1.0f}, // taille
                1.0f           // radius ignoré
            );

            _registry.emplace<RenderComponent>(
                sphereEntity,
                0x220000_rgbf,
                entityID++ // ID de l'entité pour le rendu
            );

            _registry.emplace<PhysicsLinkComponent>(
                sphereEntity,
                sphere
            );
            sphere->rigidBody().setLinearVelocity(btVector3{direction * 25.f});
        }
    }

    // const Vector3 direction = (_cameraObject->absoluteTransformation().rotationScaling() * Vector3{clickPoint, -1.0f}).normalized();
    //
    // auto* object = _physicSystem.addSphere(1,5);
    // object->translate(_cameraObject->absoluteTransformation().translation());
    // object->syncPose();
    // _renderingSystem.get()->addSphere(*object, 0.25f, 0x220000_rgbf);
    //
    // object->rigidBody().setLinearVelocity(btVector3{direction * 25.f});


    updateRegistry();
    auto packet = serializeRegistry(_registry);
    objectState->setWorld(packet);


    std::vector<Object3D*> entitesToDestroy;
    _physicSystem.update(_timeline.previousFrameDuration(), entitesToDestroy);

    for (auto obj : entitesToDestroy) {
        // remove the object from the scene
        delete obj;
        // remove the object from the registry
        auto view = _registry.view<PhysicsLinkComponent>();
        for (auto entity : view) {
            if (view.get<PhysicsLinkComponent>(entity).body == obj) {
                _registry.destroy(entity);
                break;
            }
        }
        std::cout << "Object destroyed" << std::endl;
        std::flush(std::cout);
    }


    // Here you can add code to send the state of the objects to the clients
    // For example, serialize the positions and rotations of the objects and send them over the network
    // You can use your serialization functions from Serialization.hpp
    _timeline.nextFrame();
}



int main(int argc, char** argv) {
    std::shared_ptr<Shared_Input> inputStates = std::make_shared<Shared_Input>();
    std::shared_ptr<Shared_Objects> objectStates = std::make_shared<Shared_Objects>();
    Server server;
    Magnum::Platform::Application::Arguments arguments(argc, argv);
    ServerApplication app{arguments, inputStates, objectStates};
    if (!server.start(20000))
        return -1;

    server.run(inputStates, objectStates);
    app.exec();
    //app.loop();


    //server.stop();
    return 0;
}
