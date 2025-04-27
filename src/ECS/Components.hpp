#pragma once
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/Object.h>
#include <entt/entt.hpp>

#include "../Physics/Rigidbody.h"
#include "Magnum/Math/Color.h"
#include "Magnum/Math/Quaternion.h"

using namespace Magnum;

struct TransformComponent {
    Magnum::Vector3 position;
    Magnum::Quaternion rotation;
};

struct ShapeComponent {
    enum class ShapeType { Sphere, Box };
    ShapeType type;
    float mass;
    Magnum::Vector3 size; // Utilisé pour les boîtes
    float radius; // Utilisé pour les sphères
};

struct RenderComponent {
    Magnum::Color3 color;
    float entityID; // ID de l'entité pour le rendu
    // autre info de rendu
};

struct PhysicsLinkComponent {
    RigidBody* body; // Non sérialisable, utilisé uniquement pour les calculs côté serveur
};

struct CameraComponent {
    int id; // joueur possédant la caméra
};

struct CameraLinkComponent {
    SceneGraph::Object<SceneGraph::MatrixTransformation3D>* cameraObject;
};

struct ObjectLinkComponent {
    SceneGraph::Object<SceneGraph::MatrixTransformation3D>* object;
};