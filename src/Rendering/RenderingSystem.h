#pragma once
#include <Magnum/BulletIntegration/DebugDraw.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Shaders/PhongGL.h>
#include "ColoredDrawable.h"
#include "../Physics/Rigidbody.h"

using namespace Magnum;
class RenderingSystem {
public:
    RenderingSystem();
    ~RenderingSystem();
    void render(SceneGraph::Camera3D* camera, bool _drawCubes, bool _drawDebug);

    void addBox(SceneGraph::Object<SceneGraph::MatrixTransformation3D>& object, const Vector3 size, const Color3& color);
    void addSphere(SceneGraph::Object<SceneGraph::MatrixTransformation3D>& object, const float radius, const Color3& color);

    SceneGraph::DrawableGroup3D &getDrawables() { return _drawables; }
private:
    GL::Mesh _box{NoCreate}, _sphere{NoCreate};
    GL::Buffer _boxInstanceBuffer{NoCreate}, _sphereInstanceBuffer{NoCreate};
    Shaders::PhongGL _shader{NoCreate};
    BulletIntegration::DebugDraw _debugDraw{NoCreate};
    Containers::Array<InstanceData> _boxInstanceData, _sphereInstanceData;

    SceneGraph::DrawableGroup3D _drawables;
};