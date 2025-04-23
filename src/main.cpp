#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/ImGuiIntegration/Context.hpp>
#include <Magnum/Math/Time.h>
#include <imgui.h>

#include "physics/PhysicsSystem.hpp"
#include "Rendering/RenderingSystem.h"

namespace Magnum::Game {

using namespace Math::Literals;

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

class MagnumBootstrap: public Platform::Application {
public:
    explicit MagnumBootstrap(const Arguments& arguments);

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
};

MagnumBootstrap::MagnumBootstrap(const Arguments& arguments): Platform::Application(arguments, NoCreate) {
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

    auto* ground = _physicSystem.addBox({100.0f, 0.5f, 100.0f}, 0.0f);
    _renderingSystem.get()->addBox(*ground, {100.0f, 0.5f, 100.0f}, 0xffffff_rgbf);

    Deg hue = 42.0_degf;
    for(Int i = 0; i != 5; ++i) {
        for(Int j = 0; j != 5; ++j) {
            for(Int k = 0; k != 5; ++k) {
                auto* o = _physicSystem.addBox(Vector3{0.5f});
                o->translate({i - 2.0f, j + 4.0f, k - 2.0f});
                o->syncPose();
                _renderingSystem.get()->addBox(*o, Vector3{0.5f}, Color3::fromHsv({hue += 137.5_degf, 0.75f, 0.9f}));
            }
        }
    }

    setSwapInterval(1);
    setMinimalLoopPeriod(16.0_msec);
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
    {Key::W, [this]() { _cameraObject->translateLocal(Vector3::zAxis(-0.1f)); }},
    {Key::S, [this]() { _cameraObject->translateLocal(Vector3::zAxis(0.1f)); }},
    {Key::A, [this]() { _cameraObject->translateLocal(Vector3::xAxis(-0.1f)); }},
    {Key::D, [this]() { _cameraObject->translateLocal(Vector3::xAxis(0.1f)); }},
    {Key::Q, [this]() { _cameraObject->translateLocal(Vector3::yAxis(0.1f)); }},
    {Key::E, [this]() { _cameraObject->translateLocal(Vector3::yAxis(-0.1f)); }},
    {Key::Down, [this]() { _cameraObject->rotateX(5.0_degf); }},
    {Key::Up, [this]() { _cameraObject->rotateX(-5.0_degf); }},
    {Key::Left, [this]() { _cameraRig->rotateY(-5.0_degf); }},
    {Key::Right, [this]() { _cameraRig->rotateY(5.0_degf); }},
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
    if(!event.isPrimary() || !(event.pointer() & (Pointer::MouseLeft))) return;

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

MAGNUM_APPLICATION_MAIN(Magnum::Game::MagnumBootstrap)