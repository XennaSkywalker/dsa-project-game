#define _WIN32_WINNT 0x0A00 // Fix for Windows 10 networking

#include "httplib.h"
#include "json.hpp"

// Custom Headers
#include "Player.h"
#include "GameState.h"
#include "Level.h"
#include "SaveManager.h"
#include "ReplayManager.h"
#include "TutorialManager.h"
#include "DecisionTree.h"

#include <iostream>
#include <queue>
#include <cmath>
#include <fstream>
#include <string>
#include <mutex>

using json = nlohmann::json;

// ------------------ Game State ------------------
std::mutex gameMutex;
GameState gameState;
SaveManager saveManager;
ReplayManager replayManager;
TutorialManager tutorialManager;
DecisionTree decisionTree;

std::queue<GameState> replayBackup;
bool isReplaying = false;

const double GRAVITY = 0.4;
const double JUMP = -2.0;
const double MAX_FALL = 2.0;

const int WIDTH = 50;
const int HEIGHT = 20;

Level level(WIDTH, HEIGHT);
int currentLevelID = 1;

// ------------------ Utility ------------------
std::string readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return "";
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// ------------------ Level Logic ------------------
void loadLevel(int id) {
    currentLevelID = id;
    level.resetGrid();
    replayManager.clear();
    saveManager.clear();

    gameState.player.x = 10;
    gameState.player.y = 19;
    gameState.player.vy = 0;
    gameState.player.grounded = true;

    if (id == 1) { // Tutorial
        level.createPlatform(16, 10, 10);
        level.createPlatform(13, 23, 12);
        level.addDoor(32, 12);
    }
    else if (id == 2) { // Forest
        tutorialManager.isActive = false;
        level.createPlatform(3,  15, 10);
        level.createPlatform(5,  29, 4);
        level.createPlatform(7,  37, 12);
        level.createPlatform(10, 23, 12);
        level.createPlatform(13, 10, 10);
        level.createPlatform(16, 5, 5);
        level.setGoal(15, 2);
    }
    else if (id == 3) { // Cave
        tutorialManager.isActive = false;
        level.createPlatform(3,  15, 10);
        level.createPlatform(5,  29, 4);
        level.createPlatform(7,  37, 12);
        level.createPlatform(10, 23, 12);
        level.createPlatform(13, 10, 10);
        level.createPlatform(16, 5, 5);
        level.setGoal(15, 2);
    }
}

// ------------------ Physics ------------------
void physicsTick() {
    if (!gameState.player.grounded) {
        gameState.player.vy += GRAVITY;
        if (gameState.player.vy > MAX_FALL) gameState.player.vy = MAX_FALL;
    }

    int steps = int(fabs(gameState.player.vy) + 0.5);
    int dir = (gameState.player.vy > 0) ? 1 : -1;

    for (int i = 0; i < steps; i++) {
        int newY = gameState.player.y + dir;
        if (level.isBlocked(gameState.player.x, newY)) {
            gameState.player.vy = 0;
            if (dir > 0) gameState.player.grounded = true;
            break;
        } else {
            gameState.player.y = newY;
        }
    }

    if (level.isBlocked(gameState.player.x, gameState.player.y + 1)) {
        gameState.player.grounded = true;
        gameState.player.vy = 0;
    } else {
        gameState.player.grounded = false;
    }
}

// ------------------ Input ------------------
void handleInput(const std::string& key, int choiceId = -1) {
    if (isReplaying) return;

    bool allowed = true;
    if (tutorialManager.isActive) {
        if (key == "left" || key == "right" || key == "up") {
            allowed = tutorialManager.checkProgress(key);
        }
    }
    if (!allowed) return;

    if (key == "choose" && choiceId != -1) {
        int nextLevel = decisionTree.getTargetLevel(choiceId);
        if (nextLevel != -1) {
            loadLevel(nextLevel);
            return;
        }
    }

    if (key == "left" && !level.isBlocked(gameState.player.x - 1, gameState.player.y)) {
        gameState.player.x--;
    }
    else if (key == "right" && !level.isBlocked(gameState.player.x + 1, gameState.player.y)) {
        gameState.player.x++;
    }
    else if (key == "up" && gameState.player.grounded) {
        gameState.player.vy = JUMP;
        gameState.player.grounded = false;
    }
    else if (key == "save") {
        saveManager.save(gameState);
    }
    else if (key == "undo") {
        saveManager.undo(gameState);
    }
    else if (key == "replay") {
        replayBackup = replayManager.copy();
        isReplaying = true;
    }

    else if (key == "reset")
        loadLevel(1);

    replayManager.record(gameState);
}

// ------------------ Replay Tick ------------------
void replayTick() {
    if (!isReplaying) return;
    if (replayBackup.empty()) { isReplaying = false; return; }
    gameState = replayBackup.front();
    replayBackup.pop();
}

// ------------------ Main ------------------
int main() {
    loadLevel(1);

    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::string html = readFile("index.html");
        res.set_content(html.empty() ? "<h1>Error: index.html missing</h1>" : html, "text/html");
    });

    svr.Get("/script.js", [](const httplib::Request&, httplib::Response& res) {
        std::string js = readFile("script.js");
        res.set_content(js.empty() ? "console.error('script.js missing');" : js, "application/javascript");
    });

    auto ret = svr.set_mount_point("/assets", "./assets");
    if (!ret) std::cout << "Warning: 'assets' folder not found.\n";

    svr.Post("/input", [](const httplib::Request& req, httplib::Response& res) {
        try {
            std::lock_guard<std::mutex> lock(gameMutex);
            auto j = json::parse(req.body);

            std::string key = j["key"];
            int choiceId = -1;
            if (j.contains("choiceId")) choiceId = j["choiceId"];

            handleInput(key, choiceId);
            res.set_content("{\"status\":\"ok\"}", "application/json");
        }
        catch (...) {
            res.status = 400;
        }
    });

    svr.Get("/state", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(gameMutex);

        physicsTick();
        replayTick();

        json j;

        j["player"] = { {"x", gameState.player.x}, {"y", gameState.player.y} };
        j["width"] = WIDTH;
        j["height"] = HEIGHT;

        j["tutorial"] = tutorialManager.getCurrentMessage();

        // ⭐ NEW: Goal detection message
        if (gameState.player.x == level.goalX && gameState.player.y == level.goalY) {
            j["goalMessage"] = "GOAL REACHED!";
        } else {
            j["goalMessage"] = "";
        }

        if (level.isDoor(gameState.player.x, gameState.player.y)) {
            auto options = decisionTree.getOptions();
            j["choices"] = json::array();
            for (auto& opt : options) {
                j["choices"].push_back({ {"id", opt.first}, {"text", opt.second} });
            }
        }

        j["grid"] = json::array();
        for (int y = 0; y < HEIGHT; y++) {
            std::string row = "";
            for (int x = 0; x < WIDTH; x++) {
                if (x == gameState.player.x && y == gameState.player.y) row += "P";
                else if (x == level.goalX && y == level.goalY) row += "G";
                else if (level.isDoor(x, y)) row += "D";
                else row += level.getTile(x, y);
            }
            j["grid"].push_back(row);
        }

        res.set_content(j.dump(), "application/json");
    });

    std::cout << "Server started at http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
}
