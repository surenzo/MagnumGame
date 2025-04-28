
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Time.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/ImGuiIntegration/Context.hpp>
#include "httplib.h"
#include <nlohmann/json.hpp>
#include <Corrade/Utility/Debug.h>
#include <string>

#ifdef CORRADE_TARGET_ANDROID
#include <Magnum/Platform/AndroidApplication.h>
#elif defined(CORRADE_TARGET_EMSCRIPTEN)
#include <Magnum/Platform/EmscriptenApplication.h>
#else
#include <Magnum/Platform/GlfwApplication.h>
#endif

namespace Magnum { namespace Examples {

using namespace Math::Literals;

class ImGuiExample: public Platform::Application {
    public:
        explicit ImGuiExample(const Arguments& arguments);

        void drawEvent() override;

        void viewportEvent(ViewportEvent& event) override;

        void keyPressEvent(KeyEvent& event) override;
        void keyReleaseEvent(KeyEvent& event) override;

        void pointerPressEvent(PointerEvent& event) override;
        void pointerReleaseEvent(PointerEvent& event) override;
        void pointerMoveEvent(PointerMoveEvent& event) override;
        void scrollEvent(ScrollEvent& event) override;
        void textInputEvent(TextInputEvent& event) override;

    private:
        ImGuiIntegration::Context _imgui{NoCreate};

        bool _showDemoWindow = true;
        bool _showAnotherWindow = false;
        Color4 _clearColor = 0x72909aff_rgbaf;
        Float _floatValue = 0.0f;
        std::string authToken;





    std::vector<nlohmann::json> cosmetics; // Variable pour stocker les cosmétiques
    bool isCosmeticsLoaded = false; // Flag pour savoir si les cosmétiques ont été chargés
    std::string errorMessage; // Message d'erreur si la requête échoue
        // Add member variables to store stats and achievements
    std::string statsErrorMessage;
    std::string achievementsErrorMessage;
    nlohmann::json playerStats;
    std::vector<nlohmann::json> playerAchievements;
    bool isStatsLoaded = false;
    bool isAchievementsLoaded = false;

    void login(const std::string& username, const std::string& password) {
        httplib::Client client("http://localhost:5160");
        httplib::Headers headers = {{"Content-Type", "application/json"}};
        std::string body = "{\"username\":\"" + username + "\",\"password\":\"" + password + "\"}";

        auto response = client.Post("/api/Auth/login", headers, body, "application/json");

        if (response && response->status == 200) {
            try {
                auto jsonResponse = nlohmann::json::parse(response->body);

                // Check if the "token" field exists and is not null
                if (jsonResponse.contains("token") && !jsonResponse["token"].is_null()) {
                    authToken = jsonResponse["token"].get<std::string>();
                    Corrade::Utility::Debug{} << "Login successful! Token:" << authToken.c_str();
                } else {
                    Corrade::Utility::Debug{} << "Error: 'token' field is missing or null in the response.";
                }
            } catch (const nlohmann::json::parse_error& e) {
                Corrade::Utility::Debug{} << "JSON parse error:" << e.what();
            }
        } else {
            Corrade::Utility::Debug{} << "Login failed.";
        }
    }
// Method to fetch player statistics
    void loadPlayerStats() {
        if (isStatsLoaded) return;

        httplib::Client client("http://localhost:5160");
        httplib::Headers headers = {{"Authorization", "Bearer " + authToken}};
        auto response = client.Get("/api/Statistics/statistics", headers);

        if (response) {
            if (response->status == 200) {
                try {
                    playerStats = nlohmann::json::parse(response->body);
                    isStatsLoaded = true;
                } catch (const nlohmann::json::parse_error& e) {
                    statsErrorMessage = "Failed to parse statistics data: " + std::string(e.what());
                }
            } else {
                statsErrorMessage = "Failed to load statistics. HTTP Status: " + std::to_string(response->status) +
                                    ". Response body: " + response->body;
            }
        } else {
            statsErrorMessage = "Failed to connect to the server.";
        }
    }

    void loadPlayerAchievements() {
        if (isAchievementsLoaded) return;

        httplib::Client client("http://localhost:5160");
        httplib::Headers headers = {{"Authorization", "Bearer " + authToken}};
        auto response = client.Get("/api/Statistics/achievements", headers);

        if (response) {
            if (response->status == 200) {
                try {
                    playerAchievements = nlohmann::json::parse(response->body);
                    isAchievementsLoaded = true;
                } catch (const nlohmann::json::parse_error& e) {
                    achievementsErrorMessage = "Failed to parse achievements data: " + std::string(e.what());
                }
            } else {
                achievementsErrorMessage = "Failed to load achievements. HTTP Status: " + std::to_string(response->status) +
                                           ". Response body: " + response->body;
            }
        } else {
            achievementsErrorMessage = "Failed to connect to the server.";
        }
    }

// Render the "Stats" tab
    void renderStatsTab() {
        if (!isStatsLoaded) loadPlayerStats();

        if (!statsErrorMessage.empty()) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", statsErrorMessage.c_str());
        } else {
            // Safely access JSON fields with default values
            int gamesWon = playerStats.contains("GamesWon") && !playerStats["GamesWon"].is_null()
                           ? playerStats["GamesWon"].get<int>()
                           : 0;
            int cubesCleared = playerStats.contains("CubesCleared") && !playerStats["CubesCleared"].is_null()
                               ? playerStats["CubesCleared"].get<int>()
                               : 0;
            int coins = playerStats.contains("Coins") && !playerStats["Coins"].is_null()
                        ? playerStats["Coins"].get<int>()
                        : 0;
            int cosmetics = playerStats.contains("Cosmetics") && !playerStats["Cosmetics"].is_null()
                            ? playerStats["Cosmetics"].get<int>()
                            : 0;

            ImGui::Text("Games Won: %d", gamesWon);
            ImGui::Text("Cubes Cleared: %d", cubesCleared);
            ImGui::Text("Coins: %d", coins);
            ImGui::Text("Cosmetics: %d", cosmetics);
        }
    }
// Render the "Achievement" tab
    void renderAchievementsTab() {
        if (!isAchievementsLoaded) loadPlayerAchievements();

        if (!achievementsErrorMessage.empty()) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", achievementsErrorMessage.c_str());
        } else {
            for (const auto& achievement : playerAchievements) {
                // Safely access JSON fields with default values
                std::string name = achievement.contains("Name") && !achievement["Name"].is_null()
                                   ? achievement["Name"].get<std::string>()
                                   : "Unknown";
                bool unlocked = achievement.contains("Unlocked") && !achievement["Unlocked"].is_null()
                                ? achievement["Unlocked"].get<bool>()
                                : false;

                ImGui::Text("%s: %s", name.c_str(), unlocked ? "Unlocked" : "Locked");
            }
        }
    }

    // Méthode pour récupérer les cosmétiques
    void loadCosmetics() {
        if (isCosmeticsLoaded) return; // Ne charge les cosmétiques qu'une seule fois

        httplib::Client client("http://localhost:5160");
        auto response = client.Get("/api/Store/cosmetics");

        if (response && response->status == 200) {
            try {
                cosmetics = nlohmann::json::parse(response->body);
                isCosmeticsLoaded = true; // Marquer que les cosmétiques ont été chargés
            } catch (const nlohmann::json::parse_error& e) {
                errorMessage = "Failed to parse cosmetics data.";
            }
        } else {
            errorMessage = "Failed to load cosmetics.";
        }
    }
};

ImGuiExample::ImGuiExample(const Arguments& arguments): Platform::Application{arguments,
    Configuration{}.setTitle("Magnum ImGui Example")
                   .setWindowFlags(Configuration::WindowFlag::Resizable)}
{
    _imgui = ImGuiIntegration::Context(Vector2{windowSize()}/dpiScaling(),
        windowSize(), framebufferSize());

    loadCosmetics();
    /* Set up proper blending to be used by ImGui. There's a great chance
       you'll need this exact behavior for the rest of your scene. If not, set
       this only for the drawFrame() call. */
    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add,
        GL::Renderer::BlendEquation::Add);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha,
        GL::Renderer::BlendFunction::OneMinusSourceAlpha);

    #if !defined(MAGNUM_TARGET_WEBGL) && !defined(CORRADE_TARGET_ANDROID)
    /* Have some sane speed, please */
    setMinimalLoopPeriod(16.0_msec);
    #endif
}

 enum class AppState {
    Login,
    Main
};

AppState currentState = AppState::Login;

void ImGuiExample::drawEvent() {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color);

    _imgui.newFrame();

    if (currentState == AppState::Login) {
        // Full-page login interface
        static std::string pseudo = "player1";
        static std::string password = "password111";
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
            httplib::Client client("http://localhost:5160");
            httplib::Headers headers = {{"Content-Type", "application/json"}};
            std::string body = "{\"username\":\"" + pseudo + "\",\"password\":\"" + password + "\"}";

            auto response = client.Post("/api/Auth/login", headers, body, "application/json");

            if (response && response->status == 200) {
                try {
                    auto jsonResponse = nlohmann::json::parse(response->body);
                    std::string token = jsonResponse["token"];
                    Corrade::Utility::Debug{} << "Login successful! Token:" << token.c_str();
                    login(pseudo, password); // Call the login function to set the authToken
                    currentState = AppState::Main; // Switch to the main window
                } catch (const nlohmann::json::parse_error& e) {
                    Corrade::Utility::Debug{} << "JSON parse error:" << e.what();
                    errorMessage = "Invalid server response.";
                }
            } else {
                if (response) {
                    Corrade::Utility::Debug{} << "Server response:" << response->body.c_str();
                    errorMessage = "Login failed: " + response->body;
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
                                httplib::Client client("http://localhost:5160");
                                httplib::Headers headers = {{"Content-Type", "application/json"}};
                                nlohmann::json body = {{"CosmeticId", cosmetic["id"].get<int>()}};

                                auto response = client.Post("/api/Store/buy", headers, body.dump(), "application/json");

                                if (response && response->status == 200) {
                                    Corrade::Utility::Debug{} << "Purchase successful: " << response->body.c_str();
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
    if(_imgui.handleKeyPressEvent(event)) return;
}

void ImGuiExample::keyReleaseEvent(KeyEvent& event) {
    if(_imgui.handleKeyReleaseEvent(event)) return;
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
    if(_imgui.handleTextInputEvent(event)) return;
}

}}





MAGNUM_APPLICATION_MAIN(Magnum::Examples::ImGuiExample)