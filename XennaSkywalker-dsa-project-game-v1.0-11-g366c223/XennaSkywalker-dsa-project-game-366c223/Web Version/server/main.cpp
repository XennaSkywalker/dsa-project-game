#include "httplib.h"
#include "json.hpp"

#include "Player.h"
#include "GameState.h"
#include "Level.h"
#include "SaveManager.h"
#include "ReplayManager.h"
#include "TutorialManager.h" // Include Linked List
#include "DecisionTree.h"    // Include Tree

#include <iostream>
#include <queue>
#include <cmath>
#include <fstream>

using json = nlohmann::json;

// ------------------ Game State ------------------
GameState gameState;
SaveManager saveManager;
ReplayManager replayManager;
TutorialManager tutorialManager; // The Linked List
DecisionTree decisionTree;       // The Tree

std::queue<GameState> replayBackup;
bool isReplaying = false;

const double GRAVITY = 0.4;
const double JUMP = -2.0;
const double MAX_FALL = 2.0;

const int WIDTH = 50;
const int HEIGHT = 20;

Level level(WIDTH, HEIGHT);
int currentLevelID = 1; // 1 = Tutorial/Start, 2 = Forest, 3 = Cave

// ------------------ Utility ------------------
// (Keep your existing readFile functions here)
std::string readFile(const std::string &filename) {
    std::ifstream file(filename);
    if (!file) return "";
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}
std::string readFileBinary(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) return {};
    return std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
}

// ------------------ Level Loading ------------------
void loadLevel(int id) {
    currentLevelID = id;
    level.resetGrid();
    
    // Reset Player
    gameState.player.x = 5;
    gameState.player.y = 15;
    gameState.player.vy = 0;
    gameState.player.grounded = true;

    if (id == 1) { // Tutorial Level
        level.createPlatform(18, 0, 50); // Floor
        level.createPlatform(14, 10, 10);
        level.createPlatform(11, 23, 12);
        level.setGoal(45, 17);
        
        // Add the Decision Door at the end
        level.addDoor(40, 17); 
    } 
    else if (id == 2) { // Forest Level
        // Disable tutorial for level 2
        tutorialManager.isActive = false; 
        level.createPlatform(18, 0, 50);
        level.createPlatform(15, 5, 5);
        level.createPlatform(12, 15, 5);
        level.createPlatform(9, 25, 5);
        level.setGoal(48, 5);
    }
    else if (id == 3) { // Cave Level
        // Disable tutorial for level 3
        tutorialManager.isActive = false;
        level.createPlatform(18, 0, 50);
        level.createPlatform(14, 20, 20); // Harder jumps
        level.createPlatform(10, 5, 10);
        level.setGoal(5, 5);
    }
}

// ------------------ Physics ------------------
void physicsTick() {
    if (!gameState.player.grounded) {
        gameState.player.vy += GRAVITY;
        if (gameState.player.vy > MAX_FALL) gameState.player.vy = MAX_FALL;
    }

    int steps = int(fabs(gameState.player.vy)+0.5);
    int dir = (gameState.player.vy>0)?1:-1;

    for(int i=0;i<steps;i++){
        int newY = gameState.player.y + dir;
        if(level.isBlocked(gameState.player.x,newY)){
            gameState.player.vy = 0;
            if(dir>0) gameState.player.grounded = true;
            break;
        } else gameState.player.y = newY;
    }

    if(level.isBlocked(gameState.player.x, gameState.player.y+1)){
        gameState.player.grounded = true;
        gameState.player.vy = 0;
    } else gameState.player.grounded = false;
}

// ------------------ Input ------------------
void handleInput(const std::string& key, int choiceId = -1){
    if(isReplaying) return;

    // 1. Check Tutorial Linked List
    // If tutorial is active, check if the input matches the current step
    bool allowed = true;
    if (tutorialManager.isActive) {
        // If input doesn't match the current requirement, ignore it (unless it's a system key like save)
        if (key == "left" || key == "right" || key == "up") {
            allowed = tutorialManager.checkProgress(key);
        }
    }

    if (!allowed) return; // Input rejected by tutorial

    // 2. Handle Door Choices (Tree Traversal)
    if (key == "choose" && choiceId != -1) {
        int nextLevel = decisionTree.getTargetLevel(choiceId);
        if (nextLevel != -1) {
            loadLevel(nextLevel);
            return; // Level changed, stop processing
        }
    }

    // 3. Standard Movement
    if(key=="left" && !level.isBlocked(gameState.player.x-1, gameState.player.y)) gameState.player.x--;
    else if(key=="right" && !level.isBlocked(gameState.player.x+1, gameState.player.y)) gameState.player.x++;
    else if(key=="up" && gameState.player.grounded) {
        gameState.player.vy = JUMP;
        gameState.player.grounded = false;
    }
    else if(key=="save") saveManager.save(gameState);
    else if(key=="undo") saveManager.undo(gameState);
    else if(key=="replay") {
        replayBackup = replayManager.copy();
        isReplaying = true;
    }

    replayManager.record(gameState);
}

// ------------------ Replay ------------------
void replayTick(){
    if(!isReplaying) return;
    if(replayBackup.empty()){ isReplaying=false; return; }
    gameState = replayBackup.front();
    replayBackup.pop();
}

// ------------------ Main ------------------
int main(){
    loadLevel(1); // Start at Level 1

    httplib::Server svr;

    // (Serve frontend files as before - omitted for brevity)
    // IMPORTANT: Ensure you serve your files correctly like in your original code
     svr.Get("/", [](const httplib::Request&, httplib::Response& res){
        res.set_content(readFile("./client/index.html"), "text/html");
    });
     svr.Get("/script.js", [](const httplib::Request&, httplib::Response& res){
        res.set_content(readFile("./client/script.js"), "application/javascript");
    });
    // Serve assets... (copy your previous asset logic here)


    // Input handler
    svr.Post("/input", [](const httplib::Request& req, httplib::Response& res){
        auto j = json::parse(req.body);
        std::string key = j["key"];
        int choiceId = -1;
        
        // Check if a choice ID was sent
        if (j.contains("choiceId")) {
            choiceId = j["choiceId"];
        }

        handleInput(key, choiceId);
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    // Game state endpoint
    svr.Get("/state", [](const httplib::Request&, httplib::Response& res){
        physicsTick();
        replayTick();
        
        json j;
        j["player"] = { {"x", gameState.player.x}, {"y", gameState.player.y} };
        j["width"] = WIDTH;
        j["height"] = HEIGHT;
        
        // Send Tutorial Message
        j["tutorial"] = tutorialManager.getCurrentMessage();

        // Check if player is near a door to send choices
        if (level.isDoor(gameState.player.x, gameState.player.y)) {
             std::vector<std::pair<int, std::string>> options = decisionTree.getOptions();
             j["choices"] = json::array();
             for(auto& opt : options) {
                 j["choices"].push_back({ {"id", opt.first}, {"text", opt.second} });
             }
        }

        j["grid"] = json::array();
        for(int y=0;y<HEIGHT;y++){
            std::string row="";
            for(int x=0;x<WIDTH;x++){
                if(x==gameState.player.x && y==gameState.player.y) row+="P";
                else if(x==level.goalX && y==level.goalY) row+="G";
                else row += level.getTile(x,y);
            }
            j["grid"].push_back(row);
        }

        res.set_content(j.dump(), "application/json");
    });

    std::cout << "Server running on http://localhost:8080\n";
    svr.listen("0.0.0.0",8080);
}