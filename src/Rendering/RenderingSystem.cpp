#include "RenderingSystem.h"
#include <Magnum/Trade/MeshData.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/UVSphere.h>
#include <Magnum/Math/Color.h>
#include <Magnum/SceneGraph/Camera.h>


using namespace Magnum::Math::Literals;

RenderingSystem::RenderingSystem() {
    _shader = Shaders::PhongGL{Shaders::PhongGL::Configuration{}
        .setFlags(Shaders::PhongGL::Flag::VertexColor | Shaders::PhongGL::Flag::InstancedTransformation)};
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
}

RenderingSystem::~RenderingSystem() {
    // Destructor implementation if needed
}

void RenderingSystem::render(SceneGraph::Camera3D* camera, bool _drawCubes, bool _drawDebug) {

    if(_drawCubes) {
        arrayResize(_boxInstanceData, 0);
        arrayResize(_sphereInstanceData, 0);
        camera->draw(_drawables);

        _shader.setProjectionMatrix(camera->projectionMatrix());

        _boxInstanceBuffer.setData(_boxInstanceData, GL::BufferUsage::DynamicDraw);
        _box.setInstanceCount(_boxInstanceData.size());
        _shader.draw(_box);

        _sphereInstanceBuffer.setData(_sphereInstanceData, GL::BufferUsage::DynamicDraw);
        _sphere.setInstanceCount(_sphereInstanceData.size());
        _shader.draw(_sphere);
    }
    //
    // if(_drawDebug) {
    //     if(_drawCubes)
    //         GL::Renderer::setDepthFunction(GL::Renderer::DepthFunction::LessOrEqual);
    //
    //     _debugDraw.setTransformationProjectionMatrix(
    //         _camera->projectionMatrix() * _camera->cameraMatrix());
    //     _bWorld.debugDrawWorld();
    //
    //     if(_drawCubes)
    //         GL::Renderer::setDepthFunction(GL::Renderer::DepthFunction::Less);
    // }
}

void RenderingSystem::addBox(SceneGraph::Object<SceneGraph::MatrixTransformation3D>& object, const Vector3 size, const Color3& color) {
    new ColoredDrawable{object, _boxInstanceData, color,
        Matrix4::scaling(size), _drawables};
}

void RenderingSystem::addSphere(SceneGraph::Object<SceneGraph::MatrixTransformation3D>& object, const float radius, const Color3& color) {
    new ColoredDrawable{object, _sphereInstanceData, color,
        Matrix4::scaling(Vector3{ radius}), _drawables};
}