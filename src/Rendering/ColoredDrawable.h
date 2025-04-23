#pragma once
#include <Corrade/Containers/GrowableArray.h>
#include <Magnum/Math/Color.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Drawable.h>

using namespace Magnum;

struct InstanceData {
    Matrix4 transformationMatrix;
    Matrix3x3 normalMatrix;
    Color3 color;
};

class ColoredDrawable: public SceneGraph::Drawable3D {
public:
    explicit ColoredDrawable(SceneGraph::Object<SceneGraph::MatrixTransformation3D>& object, Containers::Array<InstanceData>& instanceData, const Color3& color, const Matrix4& primitiveTransformation, SceneGraph::DrawableGroup3D& drawables)
        : SceneGraph::Drawable3D{object, &drawables}, _instanceData(instanceData), _color{color}, _primitiveTransformation{primitiveTransformation} {}

private:
    void draw(const Matrix4& transformation, SceneGraph::Camera3D&) override {
        const Matrix4 t = transformation*_primitiveTransformation;
        arrayAppend(_instanceData, InPlaceInit, t, t.normalMatrix(), _color);
    }
    Containers::Array<InstanceData>& _instanceData;
    Color3 _color;
    Matrix4 _primitiveTransformation;
};
