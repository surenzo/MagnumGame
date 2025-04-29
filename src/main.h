#pragma once
#include <Magnum/Platform/GlfwApplication.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Time.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/ImGuiIntegration/Context.hpp>
#include "httplib.h"
#include <nlohmann/json.hpp>
#include <Corrade/Utility/Debug.h>
#include <string>


namespace Magnum { namespace Examples {

using namespace Math::Literals;

class ImGuiExample: public Platform::Application {
    public:
        explicit ImGuiExample(const Arguments& arguments);

        void drawEvent() override;

        void viewportEvent(ViewportEvent& event) override;

        void keyPressEvent(KeyEvent& event) override;
        void keyReleaseEvent(KeyEvent& event) override;

        void logFromToken(std::string newToken) {
            authToken = newToken;
            loadPlayerStats();
            loadPlayerAchievements();
        }

        void pointerPressEvent(PointerEvent& event) override;
        void pointerReleaseEvent(PointerEvent& event) override;
        void pointerMoveEvent(PointerMoveEvent& event) override;
        void scrollEvent(ScrollEvent& event) override;
        void textInputEvent(TextInputEvent& event) override;
        std::string getPlayerToken() const { return authToken; }
        std::string getServerAddress() const { return _serverAddress; }

    private:
        ImGuiIntegration::Context _imgui{NoCreate};

        bool _showDemoWindow = true;
        bool _showAnotherWindow = false;
        Color4 _clearColor = 0x72909aff_rgbaf;
        Float _floatValue = 0.0f;
        std::string authToken;
        std::string _serverAddress;





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
        httplib::Client client("http://192.168.87.47:5160");
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
        Corrade::Utility::Debug{} << "Loadplayerstats" ;
        //if (isStatsLoaded) return;

        httplib::Client client("http://192.168.87.47:5160");
        httplib::Headers headers = {{"Authorization", "Bearer " + authToken}};
        auto response = client.Get("/api/Statistics/statistics", headers);

        if (response) {
            if (response->status == 200) {
                Corrade::Utility::Debug{} << response->status;
                try {
                    playerStats = nlohmann::json::parse(response->body);
                    Corrade::Utility::Debug{} << playerStats.dump(4).c_str();
                    isStatsLoaded = true;
                } catch (const nlohmann::json::parse_error& e) {
                    statsErrorMessage = "Failed to parse statistics data: " + std::string(e.what());
                }
            } else {
                Corrade::Utility::Debug{} << response->status;
                statsErrorMessage = "Failed to load statistics. HTTP Status: " + std::to_string(response->status) +
                                    ". Response body: " + response->body;
            }
        } else {
            statsErrorMessage = "Failed to connect to the server.";
        }
    }

    void loadPlayerAchievements() {
        Corrade::Utility::Debug{} << "Loadplayerachievement" ;
        //if (isAchievementsLoaded) return;

        httplib::Client client("http://192.168.87.47:5160");
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
            int gamesWon = playerStats.contains("gamesWon") && !playerStats["gamesWon"].is_null()
                           ? playerStats["gamesWon"].get<int>()
                           : 0;
            int cubesCleared = playerStats.contains("cubesCleared") && !playerStats["cubesCleared"].is_null()
                               ? playerStats["cubesCleared"].get<int>()
                               : 0;
            int coins = playerStats.contains("coins") && !playerStats["coins"].is_null()
                        ? playerStats["coins"].get<int>()
                        : 0;
            /*int cosmetics = playerStats.contains("cosmetics") && !playerStats["cosmetics"].is_null()
                            ? playerStats["cosmetics"].get<int>()
                            : 0;*/

            ImGui::Text("Games Won: %d", gamesWon);
            ImGui::Text("Cubes Cleared: %d", cubesCleared);
            ImGui::Text("Coins: %d", coins);
            //ImGui::Text("Cosmetics: %d", cosmetics);
            if (playerStats.contains("cosmetics") && playerStats["cosmetics"].is_array()) {
                ImGui::Text("Cosmetics:");
                for (const auto& cosmetic : playerStats["cosmetics"]) {
                    if (cosmetic.is_string()) {
                        ImGui::BulletText("%s", cosmetic.get<std::string>().c_str());
                    }
                }
            } else {
                ImGui::Text("Cosmetics: None");
            }
        }
    }
// Render the "Achievement" tab
    void renderAchievementsTab() {
        if (!isAchievementsLoaded) loadPlayerAchievements();

        if (!achievementsErrorMessage.empty()) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", achievementsErrorMessage.c_str());
        } else {
            ImGui::Columns(2, "AchievementsColumns", false); // Create two columns
            ImGui::Text("Achievement"); // Header for the first column
            ImGui::NextColumn();
            ImGui::Text("Status"); // Header for the second column
            ImGui::Separator(); // Add a separator between the headers and the content
            ImGui::NextColumn();
            for (const auto& achievement : playerAchievements) {
                // Safely access JSON fields with default values
                std::string name = achievement.contains("name") && !achievement["name"].is_null()
                                   ? achievement["name"].get<std::string>()
                                   : "Unknown";
                bool unlocked = achievement.contains("unlocked") && !achievement["unlocked"].is_null()
                                ? achievement["unlocked"].get<bool>()
                                : false;
                ImGui::Text("%s", name.c_str()); // Add the name to the first column
                ImGui::NextColumn();
                ImGui::Text("%s", unlocked ? "Unlocked" : "Locked"); // Add the status to the second column
                ImGui::NextColumn();

            }

            ImGui::Columns(1); // Reset to a single column layout
        }
    }

    // Méthode pour récupérer les cosmétiques
    void loadCosmetics() {
        if (isCosmeticsLoaded) return; // Ne charge les cosmétiques qu'une seule fois

        httplib::Client client("http://192.168.87.47:5160");
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
}}