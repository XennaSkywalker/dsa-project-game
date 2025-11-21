#define _WIN32_WINNT 0x0600
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <vector>
#include "json.hpp" // download from https://github.com/nlohmann/json

#pragma comment(lib,"ws2_32.lib")

using json = nlohmann::json;
using namespace std;

struct Player {
    int x, y;
    double vy;
    bool grounded;
};

class Game {
public:
    Player player;
    const int width = 60, height = 20;
    const double GRAVITY = 0.4, JUMP = -2.0, MAX_FALL = 2.0;
    vector<vector<char>> level;

    Game() {
        initLevel();
    }

    void initLevel() {
        level = vector<vector<char>>(height, vector<char>(width, ' '));
        for(int i=0;i<width;i++) level[height-1][i]='#';
        // simple platform
        for(int i=10;i<20;i++) level[14][i]='#';
        player.x = 5; player.y = height-2; player.vy=0; player.grounded=true;
    }

    bool isBlocked(int x, int y){
        if(y<0 || y>=height || x<0 || x>=width) return true;
        return level[y][x]=='#';
    }

    void updatePhysics() {
        if(!player.grounded) {
            player.vy += GRAVITY;
            if(player.vy > MAX_FALL) player.vy = MAX_FALL;
        }
        if(player.vy!=0){
            int steps = (int)(abs(player.vy)+0.5);
            int dir = (player.vy>0)?1:-1;
            for(int i=0;i<steps;i++){
                int newY = player.y + dir;
                if(isBlocked(player.x,newY)){
                    player.vy=0;
                    if(dir>0) player.grounded=true;
                    break;
                }else player.y=newY;
            }
        }
        if(isBlocked(player.x, player.y+1)) {player.grounded=true; player.vy=0;}
        else player.grounded=false;
    }

    json getState() {
        return json{
            {"x", player.x},
            {"y", player.y},
            {"vy", player.vy},
            {"grounded", player.grounded}
        };
    }

    void applyInput(const string &input){
        if(input=="left" && !isBlocked(player.x-1,player.y)) player.x--;
        else if(input=="right" && !isBlocked(player.x+1,player.y)) player.x++;
        else if(input=="jump" && player.grounded) {player.vy=JUMP; player.grounded=false;}
    }
};

// Simple HTTP server for MinGW
class HTTPServer {
    SOCKET server;
    Game &game;
public:
    HTTPServer(Game &g): game(g) {}

    void start(int port=8080) {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2,2), &wsa);
        server = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        bind(server, (sockaddr*)&addr, sizeof(addr));
        listen(server, SOMAXCONN);
        cout<<"Server running on port "<<port<<"\n";

        while(true){
            SOCKET client = accept(server, nullptr, nullptr);
            thread(&HTTPServer::handleClient, this, client).detach();
        }
    }

    void handleClient(SOCKET client){
        char buffer[1024];
        int bytes = recv(client, buffer, 1024, 0);
        if(bytes<=0){ closesocket(client); return; }

        string request(buffer, bytes);
        string method = request.substr(0, request.find(" "));

        string response;

        if(request.find("/state")!=string::npos){
            game.updatePhysics();
            json j = game.getState();
            response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + j.dump();
        } else if(request.find("/input")!=string::npos){
            // Very simple input parsing (GET /input?dir=left)
            size_t pos = request.find("dir=");
            if(pos!=string::npos){
                size_t end = request.find(' ', pos);
                string dir = request.substr(pos+4,end-(pos+4));
                game.applyInput(dir);
            }
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nOK";
        } else {
            response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nNot Found";
        }

        send(client, response.c_str(), response.size(), 0);
        closesocket(client);
    }
};

int main(){
    Game game;
    HTTPServer server(game);
    server.start(8080);
    return 0;
}
