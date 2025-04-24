#pragma once
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/SceneGraph/Object.h>
#include <entt/entt.hpp>

#include "Magnum/Math/Color.h"
#include "Magnum/Math/Quaternion.h"

using namespace Magnum;

struct Objet {
    Vector3 position;
    Quaternion rotation;
    Color3 couleur;
    enum class Forme { Boule, Cube };
    Forme forme;
};