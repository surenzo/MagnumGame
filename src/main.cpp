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

#if !defined(MAGNUM_TARGET_WEBGL) && !defined(CORRADE_TARGET_ANDROID)
        setMinimalLoopPeriod(16.0_msec);
#endif
    }

    enum class AppState {
        Login,
        Main
    };

    AppState currentState = AppState::Login;

    void ImGuiExample::drawEvent() {
        GL::Context::makeCurrent(&GL::Context::current());
        GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);

        _imgui.newFrame();

        if (currentState == AppState::Login) {
            // Full-page login interface
            static std::string pseudo = "";
            static std::string password = "";
            static std::string errorMessage;

            char pseudo_char[128];
            char password_char[128];

            std::strncpy(pseudo_char, pseudo.c_str(), sizeof(pseudo_char) - 1);
            pseudo_char[sizeof(pseudo_char) - 1] = '\0';

            std::strncpy(password_char, password.c_str(), sizeof(password_char) - 1);
            password_char[sizeof(password_char) - 1] = '\0';

            const ImVec2 windowSize = ImGui::GetIO().DisplaySize;
            const ImVec2 blockSize = ImVec2(300, 150);
            const ImVec2 blockPos = ImVec2((windowSize.x - blockSize.x) * 0.5f, (windowSize.y - blockSize.y) * 0.5f);

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(windowSize);
            ImGui::Begin("##FullPage", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

            ImGui::SetCursorPos(blockPos);

            ImGui::BeginChild("##CenteredBlock", blockSize, true, ImGuiWindowFlags_NoScrollbar);

            ImGui::Text("Pseudo:");
            ImGui::SameLine(100);
            if (ImGui::InputText("##Pseudo", pseudo_char, sizeof(pseudo_char))) {
                pseudo = std::string(pseudo_char);
            }


            ImGui::Text("Password:");
            ImGui::SameLine(100);
            if (ImGui::InputText("##Password", password_char, sizeof(password_char), ImGuiInputTextFlags_Password)) {
                password = std::string(password_char);
            }

            ImGui::SetCursorPosX((blockSize.x - 80) * 0.5f);
            if (ImGui::Button("Play", ImVec2(80, 30))) {
                httplib::Client client("http://192.168.87.47:5160");
                httplib::Headers headers = {{"Content-Type", "application/json"}};
                std::string body = "{\"username\":\"" + pseudo + "\",\"password\":\"" + password + "\"}";

                auto response = client.Post("/api/Auth/login", headers, body, "application/json");

                if (response && response->status == 200) {
                    try {
                        auto jsonResponse = nlohmann::json::parse(response->body);
                        std::string token = jsonResponse["token"];
                        Corrade::Utility::Debug{} << "Login successful! Token:" << token.c_str();
                        login(pseudo, password); // Call the login function to set the authToken
                        loadPlayerStats();
                        loadPlayerAchievements();
                        currentState = AppState::Main; // Switch to the main window
                    } catch (const nlohmann::json::parse_error& e) {
                        Corrade::Utility::Debug{} << "JSON parse error:" << e.what();
                        errorMessage = "Invalid server response.";
                    }
                } else {
                    if (response) {
                        Corrade::Utility::Debug{} << "Server response:" << response->body.c_str();
                        errorMessage = "Login failed: " + response->body;
                        Corrade::Utility::Debug{} << "Request Body:" << body.c_str();
                    } else {
                        errorMessage = "Failed to connect to the server.";
                        Corrade::Utility::Debug{} << "Login failed: No response from server.";
                    }
                }
            }

            if (!errorMessage.empty()) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", errorMessage.c_str());
            }

            ImGui::EndChild();
            ImGui::End();
        } else if (currentState == AppState::Main) {
            // Main window with tabs
            ImGui::Begin("Main Window");

            if (ImGui::BeginTabBar("Tabs")) {
                if (ImGui::BeginTabItem("Stats")) {
                    renderStatsTab();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Achievement")) {
                    renderAchievementsTab();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Shop")) {
                    if (!errorMessage.empty()) {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", errorMessage.c_str());
                    } else {
                        for (const auto& cosmetic : cosmetics) {
                            if (cosmetic.contains("name") && cosmetic.contains("description") &&
                    cosmetic.contains("price") && cosmetic.contains("id")) {
                                ImGui::Text("Name: %s", cosmetic["name"].get<std::string>().c_str());
                                ImGui::Text("Description: %s", cosmetic["description"].get<std::string>().c_str());
                                ImGui::Text("Price: %d coins", cosmetic["price"].get<int>());
                                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

                                if (ImGui::Button(("Buy " + cosmetic["name"].get<std::string>()).c_str())) {
                                    httplib::Client client("http://192.168.87.47:5160");
                                    httplib::Headers headers = {
                                        {"Content-Type", "application/json"},
                                        {"Authorization", "Bearer " + authToken} // Add the token here
                                    };
                                    nlohmann::json body = {{"CosmeticId", cosmetic["id"].get<int>()}};

                                    auto response = client.Post("/api/Store/buy", headers, body.dump(), "application/json");

                                    if (response && response->status == 200) {
                                        Corrade::Utility::Debug{} << "Purchase successful: " << response->body.c_str();
                                        loadPlayerStats();
                                        loadPlayerAchievements();
                                    } else {
                                        Corrade::Utility::Debug{} << "Purchase failed.";
                                    }
                                }
                                ImGui::Separator();
                    } else {
                        Corrade::Utility::Debug{} << "Missing keys in cosmetic JSON object. Full JSON content:" << cosmetic.dump(4).c_str();
                    }
                        }
                    }

                    ImGui::EndTabItem();
                }
                /*if (ImGui::BeginTabItem("Shop")) {
                    static std::vector<nlohmann::json> cosmetics;
                    static bool isLoaded = false;
                    static std::string errorMessage;

                    if (!isLoaded) {
                        isLoaded = true; // Marquer comme chargé avant la requête pour éviter les appels répétés
                        httplib::Client client("http://localhost:5160");
                        auto response = client.Get("/api/Store/cosmetics");

                        if (response && response->status == 200) {
                            try {
                                cosmetics = nlohmann::json::parse(response->body);
                                ImGui::Text("Vous allez jouer contre des joueurs ayant entre 5 et 15 victoires.");
                            } catch (const nlohmann::json::parse_error& e) {
                                errorMessage = "Failed to parse cosmetics data.";

                            }
                        } else {
                            errorMessage = "Failed to load cosmetics.";
                        }
                    }







                    ImGui::EndTabItem();
                }*/
                if (ImGui::BeginTabItem("Matchmaking")) {
                    ImGui::Text("Vous allez jouer contre des joueurs ayant entre 5 et 15 victoires.");

                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10); // Ajout d'un espace vertical
                    if (ImGui::Button("Play", ImVec2(100, 30))) {
                        // Action à effectuer lorsque le bouton est cliqué
                        Corrade::Utility::Debug{} << "Matchmaking Play button clicked!";

                        // Fetch the server address from the API
                        httplib::Client client("http://192.168.87.47:5160"); // Replace with your API base URL
                        httplib::Headers headers = {{"Authorization", "Bearer " + authToken}}; // Add the auth token
                        auto response = client.Post("/api/Matchmaking/join", headers);

                        if (response && response->status == 200) {
                            try {
                                auto jsonResponse = nlohmann::json::parse(response->body);
                                if (jsonResponse.contains("serverIp") && !jsonResponse["serverIp"].is_null()) {
                                    _serverAddress = jsonResponse["serverIp"].get<std::string>();
                                    Corrade::Utility::Debug{} << "Server address retrieved successfully: " << _serverAddress.c_str();
                                    exit();
                                } else {
                                    Corrade::Utility::Debug{} << "Error: 'serverIp' field is missing or null in the response.";
                                }
                            } catch (const nlohmann::json::parse_error& e) {
                                Corrade::Utility::Debug{} << "JSON parse error: " << e.what();
                            }
                        } else {
                            Corrade::Utility::Debug{} << "Failed to fetch server address. HTTP Status: "
                          << (response ? std::to_string(response->status).c_str() : "No response");
                        }
                    }

                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();

            }

            ImGui::End();
        }

        _imgui.updateApplicationCursor(*this);

        GL::Renderer::enable(GL::Renderer::Feature::Blending);
        GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
        GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
        GL::Renderer::disable(GL::Renderer::Feature::DepthTest);

        _imgui.drawFrame();

        GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
        GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
        GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
        GL::Renderer::disable(GL::Renderer::Feature::Blending);

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



//
//
// MAGNUM_APPLICATION_MAIN(Magnum::Examples::ImGuiExample)