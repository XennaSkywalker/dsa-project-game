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

const double GRAVITY = 0.4;
const double JUMP = -2.0;
const double MAX_FALL = 2.0;

const int WIDTH = 50;
const int HEIGHT = 20;

Level level(WIDTH, HEIGHT);

// ------------------ Utility ------------------
std::string readFile(const std::string &filename) {
    std::ifstream file(filename);
    if (!file) return "";
    std::string content((std::istreambuf_iterator<char>(file)),
    std::istreambuf_iterator<char>());
    return content;
}

std::string readFileBinary(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) return {};
    return std::string((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));
}  
 
// ------------------ Level Initialization ------------------
void initLevel() {
    // level.createFullGround();
    // level.createWalls();
    level.createPlatform(3,  15, 10);
    level.createPlatform(5,  29, 4);
    level.createPlatform(7,  37, 12);
    level.createPlatform(10, 23, 12);
    level.createPlatform(13, 10, 10);
    level.createPlatform(16, 5, 5);
    level.setGoal(15, 2);

    gameState.player.x = 5;
    gameState.player.y = 19;
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
        res.set_content(readFile("client\\index.html"), "text/html");
    });

    svr.Get("/script.js", [](const httplib::Request&, httplib::Response& res){
        res.set_content(readFile("client\\script.js"), "application/javascript");
    });

    // Input handler
    svr.Post("/input", [](const httplib::Request& req, httplib::Response& res){
        auto j = json::parse(req.body);
        std::string key = j["key"];
        handleInput(key);
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    // Serve game assets
    svr.Get("/assets/player.png", [](const httplib::Request&, httplib::Response& res){
        std::string content = readFileBinary("client\\assets\\player.png");
        if(content.empty()){
            res.status = 404;
            res.set_content("File not found", "text/plain");
        } else {
            res.set_content(content, "image/png");
        }
    });

    // svr.Get("/assets/wall.png", [](const httplib::Request&, httplib::Response& res){
    //     std::string content = readFileBinary("C:\\Users\\adity\\Downloads\\DSA-Project-Game\\Web Version\\client\\assets\\wall.png");
    //     if(content.empty()){
    //         res.status = 404;
    //         res.set_content("File not found", "text/plain");
    //     } else {
    //         res.set_content(content, "image/png");
    //     }
    // });

    svr.Get("/assets/platform.png", [](const httplib::Request&, httplib::Response& res){
        std::string content = readFileBinary("client\\assets\\platform.png");
        if(content.empty()){
            res.status = 404;
            res.set_content("File not found", "text/plain");
        } else {
            res.set_content(content, "image/png");
        }
    });

    svr.Get("/assets/goal.png", [](const httplib::Request&, httplib::Response& res){
        std::string content = readFileBinary("client\\assets\\goal.png");
        if(content.empty()){
            res.status = 404;
            res.set_content("File not found", "text/plain");
        } else {
            res.set_content(content, "image/png");
        }
    });

    svr.Get("/assets/top.png", [](const httplib::Request&, httplib::Response& res){
        std::string content = readFileBinary("client\\assets\\top.png");
        if(content.empty()){
            res.status = 404;
            res.set_content("File not found", "text/plain");
        } else {
            res.set_content(content, "image/png");
        }
    });

    svr.Get("/assets/background.png", [](const httplib::Request&, httplib::Response& res){
        std::string content = readFileBinary("client\\assets\\background.png");
        if(content.empty()){
            res.status = 404;
            res.set_content("File not found", "text/plain");
        } else {
            res.set_content(content, "image/png");
        }
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
