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
    Timeline _timeline;

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

    auto* dummyBox = _physicSystem.addBox(Vector3{0.5f});
    dummyBox->translate({0.0f, 0.0f, 0.0f});
    dummyBox->syncPose();
    _renderingSystem.get()->addBox(*dummyBox, Vector3{0.5f}, 0xffffff_rgbf);

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

    _physicSystem.update(_timeline.previousFrameDuration());
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
    std::cout << "Click "  << std::endl;

    const Vector2 position = event.position() * Vector2{framebufferSize()} / Vector2{windowSize()};
    const Vector2 clickPoint = Vector2::yScale(-1.0f) * (position / Vector2{framebufferSize()} - Vector2{0.5f}) * _camera->projectionSize();
    const Vector3 direction = (_cameraObject->absoluteTransformation().rotationScaling() * Vector3{clickPoint, -1.0f}).normalized();

    auto* object = _physicSystem.addSphere(1,5);
    object->translate(_cameraObject->absoluteTransformation().translation());
    object->syncPose();
    _renderingSystem.get()->addSphere(*object, 0.25f, 0x220000_rgbf);

    object->rigidBody().setLinearVelocity(btVector3{direction * 25.f});

    event.setAccepted();
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
