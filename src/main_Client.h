#pragma once

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
#include "ECS/Deserialization.cpp"

namespace Magnum::Game {

    using namespace Math::Literals;

    typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
    typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

    class MagnumBootstrap: public Platform::Application {
    public:
        explicit MagnumBootstrap(const Arguments& arguments, std::shared_ptr<Shared_Input> inputStates, std::shared_ptr<Shared_Objects> objectStates);
        void setPlayer(int player) { this->player = player; }

    private:
        void pointerMoveEvent(PointerMoveEvent &event);

        void drawEvent() override;
        void viewportEvent(ViewportEvent& event) override;
        void keyPressEvent(KeyEvent& event) override;
        void keyReleaseEvent(KeyEvent &event) override;
        void pointerPressEvent(PointerEvent& event) override;
        void pointerReleaseEvent(PointerEvent &event) override ;

        void updateRegistry(const entt::registry& newRegistry);
        Timeline _timeline;

        entt::registry _registry;
        std::unordered_map<uint32_t,entt::entity> linkingContext;


        std::unique_ptr<RenderingSystem> _renderingSystem;

        Scene3D _scene;
        Object3D *_cameraRig, *_cameraObject;
        SceneGraph::Camera3D* _camera;

        bool _drawCubes{true}, _drawDebug{true};
        ImGuiIntegration::Context _imgui{NoCreate};
        std::shared_ptr<Shared_Input> inputState;
        std::shared_ptr<Shared_Objects> objectState;
        bool _showAnotherWindow = false;

        enum gameState {
            PLAYING,
            GAME_OVER
        };
        gameState _gameState = PLAYING;

        int player;
    };

}