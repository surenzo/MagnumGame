#include "main.h"

namespace Magnum { namespace Examples {

using namespace Math::Literals;


    ImGuiExample::ImGuiExample(const Arguments& arguments) : Platform::Application{arguments,
        Configuration{}.setTitle("Magnum ImGui Example")
                       .setWindowFlags(Configuration::WindowFlag::Resizable)} {
        // Enable text input
        startTextInput();

        _imgui = ImGuiIntegration::Context(Vector2{windowSize()}/dpiScaling(),
            windowSize(), framebufferSize());

        loadCosmetics();


        GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
            GL::Renderer::BlendEquation::Add);
        GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
            GL::Renderer::BlendFunction::OneMinusSourceAlpha);

    }



void ImGuiExample::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);

    _imgui.newFrame();

    ImGui::SetNextWindowSize(ImVec2(500, 100), ImGuiCond_FirstUseEver);
    ImGui::Begin("Fin de partie", &_showAnotherWindow);
    ImGui::Text("Victoire");
    ImGui::End();

    /* Update application cursor */
    _imgui.updateApplicationCursor(*this);

    /* Set appropriate states. If you only draw ImGui, it is sufficient to
       just enable blending and scissor test in the constructor. */
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::disable(GL::Renderer::Feature::DepthTest);

    _imgui.drawFrame();


    swapBuffers();
    redraw();

}

void ImGuiExample::viewportEvent(ViewportEvent& event) {
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});

    _imgui.relayout(Vector2{event.windowSize()}/event.dpiScaling(),
        event.windowSize(), event.framebufferSize());
}

void ImGuiExample::keyPressEvent(KeyEvent& event) {
    if (_imgui.handleKeyPressEvent(event)) {
        event.setAccepted();
        return;
    }
}

void ImGuiExample::keyReleaseEvent(KeyEvent& event) {
    if (_imgui.handleKeyReleaseEvent(event)) {
        event.setAccepted(); // Marque l'événement comme traité
        return;
    }
}

void ImGuiExample::pointerPressEvent(PointerEvent& event) {
    if(_imgui.handlePointerPressEvent(event)) return;
}

void ImGuiExample::pointerReleaseEvent(PointerEvent& event) {
    if(_imgui.handlePointerReleaseEvent(event)) return;
}

void ImGuiExample::pointerMoveEvent(PointerMoveEvent& event) {
    if(_imgui.handlePointerMoveEvent(event)) return;
}

void ImGuiExample::scrollEvent(ScrollEvent& event) {
    if(_imgui.handleScrollEvent(event)) {
        /* Prevent scrolling the page */
        event.setAccepted();
        return;
    }
}

    void ImGuiExample::textInputEvent(TextInputEvent& event) {

    // Transmit the event to ImGui
    if (_imgui.handleTextInputEvent(event)) {
        event.setAccepted(); // Mark the event as handled
        return;
    }

}
}

}
