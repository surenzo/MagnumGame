#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/GL/Mesh.h>
#include <btBulletDynamicsCommon.h>
#include <Magnum/BulletIntegration/Integration.h>
#include <Magnum/BulletIntegration/MotionState.h>
#include <Magnum/BulletIntegration/DebugDraw.h>
#include <random>

#include "Corrade/Containers/Pointer.h"

using namespace Magnum;
using namespace Math::Literals;
typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;
class ColoredDrawable : public SceneGraph::Drawable3D {
public:
    explicit ColoredDrawable(Object3D& object, Shaders::Phong& shader, GL::Mesh& mesh, const Color3& color, SceneGraph::DrawableGroup3D& group) :
        SceneGraph::Drawable3D{object, &group}, _shader(shader), _mesh(mesh), _color(color) {}

    void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override {
        _shader.setLightPosition({-25.0f, 70.0f, 5.5f})
        .setAmbientColor(0x222222_rgbf) // Set ambient color
        .setDiffuseColor(_color)
        .setSpecularColor(0xffffff_rgbf)
        .setShininess(50.0f)
               .setTransformationMatrix(transformationMatrix)
               .setNormalMatrix(transformationMatrix.normalMatrix())
               .setProjectionMatrix(camera.projectionMatrix());
        _mesh.draw(_shader);
    }

private:
    Shaders::Phong& _shader;
    GL::Mesh& _mesh;
    Color3 _color;
};

class RigidBody : public Object3D {
public:
    RigidBody(Object3D* parent, Float mass, btCollisionShape* bShape, btDynamicsWorld& bWorld)
        : Object3D{parent}, _bWorld(bWorld) {
        btVector3 bInertia(0.0f, 0.0f, 0.0f);
        if(!Math::TypeTraits<Float>::equals(mass, 0.0f))
            bShape->calculateLocalInertia(mass, bInertia);

        auto* motionState = new BulletIntegration::MotionState{*this};
        _bRigidBody.emplace(btRigidBody::btRigidBodyConstructionInfo{
            mass, &motionState->btMotionState(), bShape, bInertia});
        _bRigidBody->forceActivationState(DISABLE_DEACTIVATION);
        bWorld.addRigidBody(_bRigidBody.get());
    }

    ~RigidBody() {
        _bWorld.removeRigidBody(_bRigidBody.get());
    }

    btRigidBody& rigidBody() { return *_bRigidBody; }

    void syncPose() {
        _bRigidBody->setWorldTransform(btTransform(transformationMatrix()));
    }

private:
    btDynamicsWorld& _bWorld;
    Containers::Pointer<btRigidBody> _bRigidBody;
};

Color3 randomColor() {
    static std::mt19937 gen{std::random_device{}()};
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return {dist(gen), dist(gen), dist(gen)};
}


void createLargeCube(Object3D* parent, btDynamicsWorld& bWorld, SceneGraph::DrawableGroup3D& drawables, Shaders::Phong& shader, GL::Mesh& mesh) {
    btBoxShape* smallBoxShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
    for(int x = 0; x < 5; ++x) {
        for(int y = 0; y < 5; ++y) {
            for(int z = 0; z < 5; ++z) {
                RigidBody* smallCube = new RigidBody{parent, 1.0f, smallBoxShape, bWorld};
                smallCube->translate(Vector3(x * 1.2f, y * 1.1f, z * 1.2f)); // Slightly increase spacing to avoid overlap
                smallCube->syncPose();
                new ColoredDrawable{*smallCube, shader, mesh, randomColor(), drawables}; // Utilisez une couleur aléatoire
            }
        }
    }
}

void shootCube(Object3D* parent, btDynamicsWorld& bWorld, SceneGraph::DrawableGroup3D& drawables, Shaders::Phong& shader, GL::Mesh& mesh, const Vector3& position, const Vector3& direction) {
    btBoxShape* smallBoxShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
    RigidBody* smallCube = new RigidBody{parent, 2.0f, smallBoxShape, bWorld};
    smallCube->translate(position);
    smallCube->syncPose();
    new ColoredDrawable{*smallCube, shader, mesh, 0xff0000_rgbf, drawables};

    btVector3 velocity(direction.x(), direction.y(), direction.z());
    velocity.normalize();
    velocity *= 60.0f; // Set the speed of the cube
    smallCube->rigidBody().setLinearVelocity(velocity);
}

void shootSphere(Object3D* parent, btDynamicsWorld& bWorld, SceneGraph::DrawableGroup3D& drawables, Shaders::Phong& shader, GL::Mesh& mesh, const Vector3& position, const Vector3& direction) {
    btSphereShape* sphereShape = new btSphereShape(0.5f); // Create a sphere shape with radius 0.5
    RigidBody* sphere = new RigidBody{parent, 2.0f, sphereShape, bWorld};
    sphere->translate(position);
    sphere->syncPose();
    new ColoredDrawable{*sphere, shader, mesh, 0xff0000_rgbf, drawables};

    btVector3 velocity(direction.x(), direction.y(), direction.z());
    velocity.normalize();
    velocity *= 60.0f; // Set the speed of the sphere
    sphere->rigidBody().setLinearVelocity(velocity);
}

class MagnumBootstrap : public Platform::Application {
public:
    explicit MagnumBootstrap(const Arguments& arguments);

private:
    void drawEvent() override;
    void keyPressEvent(KeyEvent& event) override;
    void mousePressEvent(MouseEvent& event) override;

    Scene3D _scene;
    SceneGraph::DrawableGroup3D _drawables;
    RigidBody* _cube;
    RigidBody* _groundBody;
    Object3D *_cameraRig, *_cameraObject;
    Object3D _ground;

    SceneGraph::Camera3D* _camera;
    Shaders::Phong _shader;
    GL::Mesh _mesh, _groundMesh;
    btDiscreteDynamicsWorld* _bWorld;
    btDefaultCollisionConfiguration* _collisionConfig;
    btCollisionDispatcher* _dispatcher;
    btDbvtBroadphase* _broadphase;
    btSequentialImpulseConstraintSolver* _solver;
    btBoxShape _bBoxShape;
    btBoxShape _groundShape;
};

MagnumBootstrap::MagnumBootstrap(const Arguments& arguments) :
    Platform::Application{arguments, Configuration{}.setTitle("Magnum Bootstrap")},
    _collisionConfig{new btDefaultCollisionConfiguration()},
    _dispatcher{new btCollisionDispatcher(_collisionConfig)},
    _broadphase{new btDbvtBroadphase()},
    _solver{new btSequentialImpulseConstraintSolver()},
    _bBoxShape{btVector3(0.5f, 0.5f, 0.5f)},
    _groundShape{btVector3(20.0f, 0.05f, 20.0f)}{

    _bWorld = new btDiscreteDynamicsWorld(_dispatcher, _broadphase, _solver, _collisionConfig);

    _mesh = MeshTools::compile(Primitives::cubeSolid());
    _groundMesh = MeshTools::compile(Primitives::cubeSolid());

    _groundBody = new RigidBody{&_scene, 0.0f, &_groundShape, *_bWorld}; // Ground with mass 0
    _groundBody->scale(Vector3(10.0f, 0.1f, 10.0f)); // Scale the ground to be larger and flat
    new ColoredDrawable{*_groundBody, _shader, _groundMesh, 0xffffff_rgbf, _drawables};
    createLargeCube(&_scene, *_bWorld, _drawables, _shader, _mesh);
    /* Camera setup */
    (*(_cameraRig = new Object3D{&_scene}))
        .translate(Vector3::yAxis(3.0f))
        .rotateY(40.0_degf);
    (*(_cameraObject = new Object3D{_cameraRig}))
        .translate(Vector3::zAxis(50.0f))
        .rotateX(-25.0_degf);
    (_camera = new SceneGraph::Camera3D(*_cameraObject))
        ->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
        .setViewport(GL::defaultFramebuffer.viewport().size());

    Vector2i size = windowSize();
    float aspectRatio = float(size.x()) / float(size.y());
    _camera->setProjectionMatrix(Matrix4::perspectiveProjection(
        35.0_degf, aspectRatio, 1.0f, 1000.0f));
}


void MagnumBootstrap::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    _bWorld->stepSimulation(1.0f / 120.0f); // Step the physics simulation

    _camera->draw(_drawables);

    swapBuffers();
    redraw();
}

/*void MagnumBootstrap::keyPressEvent(KeyEvent& event) {

    if(event.key() == Key::Down) {
        _cameraObject->rotateX(5.0_degf);
    } else if(event.key() == Key::Up) {
        _cameraObject->rotateX(-5.0_degf);
    } else if(event.key() == Key::Left) {
        _cameraRig->rotateY(-5.0_degf);
    } else if(event.key() == Key::Right) {
        _cameraRig->rotateY(5.0_degf);
    }
}*/

void MagnumBootstrap::keyPressEvent(KeyEvent& event) {
    const float moveSpeed = 0.5f;
    Math::Deg<float> rotateSpeed = 5.0_degf;

    if(event.key() == Key::W) {
        _cameraObject->translateLocal(Vector3::zAxis(-moveSpeed));
    } else if(event.key() == Key::S) {
        _cameraObject->translateLocal(Vector3::zAxis(moveSpeed));
    } else if(event.key() == Key::A) {
        _cameraObject->translateLocal(Vector3::xAxis(-moveSpeed));
    } else if(event.key() == Key::D) {
        _cameraObject->translateLocal(Vector3::xAxis(moveSpeed));
    } else if(event.key() == Key::Q) {
        _cameraObject->translateLocal(Vector3::yAxis(-moveSpeed));
    } else if(event.key() == Key::E) {
        _cameraObject->translateLocal(Vector3::yAxis(moveSpeed));
    } else if(event.key() == Key::Down) {
        _cameraObject->rotateX(rotateSpeed);
    } else if(event.key() == Key::Up) {
        _cameraObject->rotateX(-rotateSpeed);
    } else if(event.key() == Key::Left) {
        _cameraRig->rotateY(-rotateSpeed);
    } else if(event.key() == Key::Right) {
        _cameraRig->rotateY(rotateSpeed);
    }
}

void MagnumBootstrap::mousePressEvent(MouseEvent& event) {
    if(event.button() == MouseEvent::Button::Left) {
        Vector2 position = Vector2{event.position()} * Vector2{Vector2{framebufferSize()}} / Vector2{Vector2{windowSize()}};
        Vector2 clickPoint = Vector2::yScale(-1.0f) * (position / Vector2{Vector2{framebufferSize()}} - Vector2{0.5f}) * _camera->projectionSize();
        Vector3 direction = (_cameraObject->absoluteTransformation().rotationScaling() * Vector3{clickPoint, -1.0f}).normalized();

        Vector3 cameraPosition = _cameraObject->absoluteTransformation().translation();
        shootCube(&_scene, *_bWorld, _drawables, _shader, _mesh, cameraPosition, direction);
    }
    if(event.button() == MouseEvent::Button::Right) {
        Vector2 position = Vector2{event.position()} * Vector2{Vector2{framebufferSize()}} / Vector2{Vector2{windowSize()}};
        Vector2 clickPoint = Vector2::yScale(-1.0f) * (position / Vector2{Vector2{framebufferSize()}} - Vector2{0.5f}) * _camera->projectionSize();
        Vector3 direction = (_cameraObject->absoluteTransformation().rotationScaling() * Vector3{clickPoint, -1.0f}).normalized();

        Vector3 cameraPosition = _cameraObject->absoluteTransformation().translation();
        shootSphere(&_scene, *_bWorld, _drawables, _shader, _mesh, cameraPosition, direction);
    }
}
MAGNUM_APPLICATION_MAIN(MagnumBootstrap)