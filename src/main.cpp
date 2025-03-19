#include <btBulletDynamicsCommon.h>
#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Containers/Optional.h>
#include <Corrade/Containers/Pointer.h>
#include <Magnum/Timeline.h>
#include <Magnum/BulletIntegration/Integration.h>
#include <Magnum/BulletIntegration/MotionState.h>
#include <Magnum/BulletIntegration/DebugDraw.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Constants.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Time.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/MeshTools/Transform.h>
#ifdef CORRADE_TARGET_EMSCRIPTEN
#include <Magnum/Platform/EmscriptenApplication.h>
#else
#endif
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/Shaders/PhongGL.h>
#include <Magnum/Trade/MeshData.h>

#include "Magnum/Platform/GlfwApplication.h"





using namespace Magnum;
using namespace Math::Literals;
namespace Magnum {
    namespace Examples {
        typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
        typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

        struct InstanceData {
            Matrix4 transformationMatrix;
            Matrix3x3 normalMatrix;
            Color3 color;
        };

        class MagnumBootstrap: public Platform::Application {
        public:
            explicit MagnumBootstrap(const Arguments& arguments);

        private:
            void drawEvent() override;
            void viewportEvent(ViewportEvent& event) override;
            void keyPressEvent(KeyEvent& event) override;
            void pointerPressEvent(PointerEvent& event) override;

            GL::Mesh _box{NoCreate}, _sphere{NoCreate};
            GL::Buffer _boxInstanceBuffer{NoCreate}, _sphereInstanceBuffer{NoCreate};
            Shaders::PhongGL _shader{NoCreate};
            BulletIntegration::DebugDraw _debugDraw{NoCreate};
            Containers::Array<InstanceData> _boxInstanceData, _sphereInstanceData;

            btDbvtBroadphase _bBroadphase;
            btDefaultCollisionConfiguration _bCollisionConfig;
            btCollisionDispatcher _bDispatcher{&_bCollisionConfig};
            btSequentialImpulseConstraintSolver _bSolver;


            btDiscreteDynamicsWorld _bWorld{&_bDispatcher, &_bBroadphase, &_bSolver, &_bCollisionConfig};

            Scene3D _scene;
            SceneGraph::Camera3D* _camera;
            SceneGraph::DrawableGroup3D _drawables;
            Timeline _timeline;

            Object3D *_cameraRig, *_cameraObject;

            btBoxShape _bBoxShape{{0.5f, 0.5f, 0.5f}};
            btSphereShape _bSphereShape{0.25f};
            btBoxShape _bGroundShape{{4.0f, 0.5f, 4.0f}};

            bool _drawCubes{true}, _drawDebug{true}, _shootBox{true};
        };

        class ColoredDrawable: public SceneGraph::Drawable3D {
        public:
            explicit ColoredDrawable(Object3D& object, Containers::Array<InstanceData>& instanceData, const Color3& color, const Matrix4& primitiveTransformation, SceneGraph::DrawableGroup3D& drawables): SceneGraph::Drawable3D{object, &drawables}, _instanceData(instanceData), _color{color}, _primitiveTransformation{primitiveTransformation} {}

        private:
            void draw(const Matrix4& transformation, SceneGraph::Camera3D&) override {
                const Matrix4 t = transformation*_primitiveTransformation;
                arrayAppend(_instanceData, InPlaceInit, t, t.normalMatrix(), _color);
            }

            Containers::Array<InstanceData>& _instanceData;
            Color3 _color;
            Matrix4 _primitiveTransformation;
        };

        class RigidBody: public Object3D {
        public:
            RigidBody(Object3D* parent, Float mass, btCollisionShape* bShape, btDynamicsWorld& bWorld): Object3D{parent}, _bWorld(bWorld) {

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

        MagnumBootstrap::MagnumBootstrap(const Arguments& arguments): Platform::Application(arguments, NoCreate) {

            {
                const Vector2 dpiScaling = this->dpiScaling({});
                Configuration conf;
                conf.setTitle("Magnum Bullet Integration Example")
                    .setWindowFlags(Configuration::WindowFlag::Resizable)
                    .setSize(conf.size(), dpiScaling);
                GLConfiguration glConf;
                glConf.setSampleCount(dpiScaling.max() < 2.0f ? 8 : 2);
                if(!tryCreate(conf, glConf))
                    create(conf, glConf.setSampleCount(0));
            }

            /* Camera setup */
            (*(_cameraRig = new Object3D{&_scene}))
                .translate(Vector3::yAxis(3.0f))
                .rotateY(40.0_degf);
            (*(_cameraObject = new Object3D{_cameraRig}))
                .translate(Vector3::zAxis(20.0f))
                .rotateX(-25.0_degf);
            (_camera = new SceneGraph::Camera3D(*_cameraObject))
                ->setAspectRatioPolicy(SceneGraph::AspectRatioPolicy::Extend)
                .setProjectionMatrix(Matrix4::perspectiveProjection(35.0_degf, 1.0f, 0.001f, 100.0f))
                .setViewport(GL::defaultFramebuffer.viewport().size());


            _shader = Shaders::PhongGL{Shaders::PhongGL::Configuration{}
                .setFlags(Shaders::PhongGL::Flag::VertexColor|
                          Shaders::PhongGL::Flag::InstancedTransformation)};
            _shader.setAmbientColor(0x222222_rgbf)
                   .setDiffuseColor(0x888888_rgbf)
                   .setSpecularColor(0xffffff_rgbf)
                   .setShininess(50.0f)
                   .setLightPositions({{10.0f, 15.0f, 5.0f, 0.0f}});


            _box = MeshTools::compile(Primitives::cubeSolid());
            _sphere = MeshTools::compile(Primitives::uvSphereSolid(16, 32));
            _boxInstanceBuffer = GL::Buffer{};
            _sphereInstanceBuffer = GL::Buffer{};
            _box.addVertexBufferInstanced(_boxInstanceBuffer, 1, 0,
                Shaders::PhongGL::TransformationMatrix{},
                Shaders::PhongGL::NormalMatrix{},
                Shaders::PhongGL::Color3{});
            _sphere.addVertexBufferInstanced(_sphereInstanceBuffer, 1, 0,
                Shaders::PhongGL::TransformationMatrix{},
                Shaders::PhongGL::NormalMatrix{},
                Shaders::PhongGL::Color3{});


            GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
            GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
            GL::Renderer::enable(GL::Renderer::Feature::PolygonOffsetFill);
            GL::Renderer::setPolygonOffset(2.0f, 0.5f);

            _debugDraw = BulletIntegration::DebugDraw{};
            _debugDraw.setMode(BulletIntegration::DebugDraw::Mode::DrawWireframe);
            _bWorld.setGravity({0.0f, -10.0f, 0.0f});
            _bWorld.setDebugDrawer(&_debugDraw);

            auto* ground = new RigidBody{&_scene, 0.0f, &_bGroundShape, _bWorld};
            new ColoredDrawable{*ground, _boxInstanceData, 0xffffff_rgbf,
                Matrix4::scaling({4.0f, 0.5f, 4.0f}), _drawables};


            Deg hue = 42.0_degf;
            for(Int i = 0; i != 5; ++i) {
                for(Int j = 0; j != 5; ++j) {
                    for(Int k = 0; k != 5; ++k) {
                        auto* o = new RigidBody{&_scene, 1.0f, &_bBoxShape, _bWorld};
                        o->translate({i - 2.0f, j + 4.0f, k - 2.0f});
                        o->syncPose();
                        new ColoredDrawable{*o, _boxInstanceData,
                            Color3::fromHsv({hue += 137.5_degf, 0.75f, 0.9f}),
                            Matrix4::scaling(Vector3{0.5f}), _drawables};
                    }
                }
            }


#ifndef CORRADE_TARGET_EMSCRIPTEN
            setSwapInterval(1);
            setMinimalLoopPeriod(16.0_msec);
#endif
            _timeline.start();
        }

        void MagnumBootstrap::viewportEvent(ViewportEvent& event) {
            GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});
        }

        void MagnumBootstrap::drawEvent() {
            GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);


            for(Object3D* obj = _scene.children().first(); obj; )
            {
                Object3D* next = obj->nextSibling();
                if(obj->transformation().translation().dot() > 100*100)
                    delete obj;

                obj = next;
            }


            _bWorld.stepSimulation(_timeline.previousFrameDuration(), 5);

            if(_drawCubes) {

                arrayResize(_boxInstanceData, 0);
                arrayResize(_sphereInstanceData, 0);
                _camera->draw(_drawables);

                _shader.setProjectionMatrix(_camera->projectionMatrix());


                _boxInstanceBuffer.setData(_boxInstanceData, GL::BufferUsage::DynamicDraw);
                _box.setInstanceCount(_boxInstanceData.size());
                _shader.draw(_box);

                _sphereInstanceBuffer.setData(_sphereInstanceData, GL::BufferUsage::DynamicDraw);
                _sphere.setInstanceCount(_sphereInstanceData.size());
                _shader.draw(_sphere);
            }


            if(_drawDebug) {
                if(_drawCubes)
                    GL::Renderer::setDepthFunction(GL::Renderer::DepthFunction::LessOrEqual);

                _debugDraw.setTransformationProjectionMatrix(
                    _camera->projectionMatrix()*_camera->cameraMatrix());
                _bWorld.debugDrawWorld();

                if(_drawCubes)
                    GL::Renderer::setDepthFunction(GL::Renderer::DepthFunction::Less);
            }

            swapBuffers();
            _timeline.nextFrame();
            redraw();
        }

        void MagnumBootstrap::keyPressEvent(KeyEvent& event) {
            if(event.key() == Key::W) {
                _cameraObject->translateLocal(Vector3::zAxis(-0.1f));
            } else if(event.key() == Key::S) {
                _cameraObject->translateLocal(Vector3::zAxis(0.1f));
            } else if(event.key() == Key::A) {
                _cameraObject->translateLocal(Vector3::xAxis(-0.1f));
            } else if(event.key() == Key::D) {
                _cameraObject->translateLocal(Vector3::xAxis(0.1f));
            } else if(event.key() == Key::Q) {
                _cameraObject->translateLocal(Vector3::yAxis(0.1f));
            } else if(event.key() == Key::E) {
                _cameraObject->translateLocal(Vector3::yAxis(-0.1f));
            } else if(event.key() == Key::Down) {
                _cameraObject->rotateX(5.0_degf);
            } else if(event.key() == Key::Up) {
                _cameraObject->rotateX(-5.0_degf);
            } else if(event.key() == Key::Left) {
                _cameraRig->rotateY(-5.0_degf);
            } else if(event.key() == Key::Right) {
                _cameraRig->rotateY(5.0_degf);
            } else if(event.key() == Key::C) {
                if(_drawCubes && _drawDebug) {
                    _drawDebug = false;
                } else if(_drawCubes && !_drawDebug) {
                    _drawCubes = false;
                    _drawDebug = true;
                } else if(!_drawCubes && _drawDebug) {
                    _drawCubes = true;
                    _drawDebug = true;
                }
            } else if(event.key() == Key::S) {
                _shootBox ^= true;
            } else return;

            event.setAccepted();
        }

        void MagnumBootstrap::pointerPressEvent(PointerEvent& event) {

            if(!event.isPrimary() || !(event.pointer() & Pointer::MouseLeft))
                return;


            const Vector2 position = event.position() * Vector2{framebufferSize()} / Vector2{windowSize()};
            const Vector2 clickPoint = Vector2::yScale(-1.0f) * (position / Vector2{framebufferSize()} - Vector2{0.5f}) * _camera->projectionSize();
            const Vector3 direction = (_cameraObject->absoluteTransformation().rotationScaling() * Vector3{clickPoint, -1.0f}).normalized();

            auto* object = new RigidBody{
                &_scene,
                _shootBox ? 1.0f : 5.0f,
                _shootBox ? static_cast<btCollisionShape*>(&_bBoxShape) : &_bSphereShape,
                _bWorld};
            object->translate(_cameraObject->absoluteTransformation().translation());
            object->syncPose();

            /* Create either a box or a sphere */
            new ColoredDrawable{*object,
                _shootBox ? _boxInstanceData : _sphereInstanceData,
                _shootBox ? 0x880000_rgbf : 0x220000_rgbf,
                Matrix4::scaling(Vector3{_shootBox ? 0.5f : 0.25f}), _drawables};

            /* Give it an initial velocity */
            object->rigidBody().setLinearVelocity(btVector3{direction * 25.f});

            event.setAccepted();
        }
    }
}
MAGNUM_APPLICATION_MAIN(Magnum::Examples::MagnumBootstrap)