#pragma once
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/Object.h>
#include <entt/entt.hpp>

using namespace Magnum;

// Ton composant pour les objets 3D
struct Object3D {
    SceneGraph::Object<SceneGraph::MatrixTransformation3D>* object;
};

// Composant pour la physique
struct RigidBodyComponent {
    btRigidBody* body;
};