#include "httplib.h"
#include "json.hpp"

#include "Player.h"
#include "GameState.h"
#include "Level.h"
#include "SaveManager.h"
#include "ReplayManager.h"

#include <iostream>
#include <queue>
#include <cmath>
#include <fstream>

using json = nlohmann::json;

// ------------------ Game State ------------------
GameState gameState;
SaveManager saveManager;
ReplayManager replayManager;
std::queue<GameState> replayBackup;
bool isReplaying = false;

// Physics parameters
const double GRAVITY = 0.4;
const double JUMP = -2.0;
const double MAX_FALL = 2.0;

// Grid dimensions
const int WIDTH = 60;
const int HEIGHT = 20;

// Simple level
Level level(WIDTH, HEIGHT);

// ------------------ Utility ------------------
std::string readFile(const std::string &filename) {
    std::ifstream file(filename);
    if (!file) return "";
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    return content;
}

// ------------------ Level Initialization ------------------
void initLevel() {
    level.createFullGround();
    level.createWalls();
    level.createPlatform(14, 10, 10);
    level.createPlatform(11, 23, 12);
    level.createPlatform(8,  37, 13);
    level.createPlatform(7,  29, 4);
    level.createPlatform(5,  15, 10);
    level.createPlatform(17, 5, 5);
    level.setGoal(15, 4);

    gameState.player.x = 2;
    gameState.player.y = HEIGHT-2;
    gameState.player.vy = 0;
    gameState.player.grounded = true;
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
void handleInput(const std::string& key){
    if(isReplaying) return;

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
    initLevel();

    httplib::Server svr;

    // Serve frontend files
    svr.Get("/", [](const httplib::Request&, httplib::Response& res){
        res.set_content(readFile("C:\\Users\\adity\\Downloads\\DSA-Project-Game\\Web Version\\client\\index.html"), "text/html");
    });

    svr.Get("/script.js", [](const httplib::Request&, httplib::Response& res){
        res.set_content(readFile("C:\\Users\\adity\\Downloads\\DSA-Project-Game\\Web Version\\client\\script.js"), "application/javascript");
    });

    // Input handler
    svr.Post("/input", [](const httplib::Request& req, httplib::Response& res){
        auto j = json::parse(req.body);
        std::string key = j["key"];
        handleInput(key);
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
