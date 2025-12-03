#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <cmath>

using namespace std;

struct Player {
    int x, y;
    double vy;
    bool grounded;
};

class Game {
private:
    vector<vector<char>> level;
    Player player;
    int width, height;
    const double GRAVITY = 0.4;
    const double JUMP = -2.0;
    const double MAX_FALL = 2.0;
    HANDLE hConsole;

public:
    Game() {
        width = 60;
        height = 20;
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        
        // Set buffer size to match window
        COORD bufferSize = {(SHORT)(width + 3), (SHORT)(height + 4)};
        SetConsoleScreenBufferSize(hConsole, bufferSize);
        
        // Set window size
        SMALL_RECT windowSize = {0, 0, (SHORT)(width + 2), (SHORT)(height + 3)};
        SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
        
        // Hide cursor
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
        
        SetConsoleTitle("Platformer Game");
        
        initLevel();
    }

    void initLevel() {
        level = vector<vector<char>>(height, vector<char>(width, ' '));
        
        // Ground
        for (int i = 0; i < width; i++) {
            level[height-1][i] = '#';
        }
        
        // Platforms - spaced out vertically
        for (int i = 10; i < 20; i++) level[14][i] = '#';
        for (int i = 23; i < 35; i++) level[11][i] = '#';
        for (int i = 37; i < 50; i++) level[8][i] = '#';
        for (int i = 29; i < 33; i++) level[7][i] = '#';
        for (int i = 15; i < 25; i++) level[5][i] = '#';
        
        // Small step platforms on left
        for (int i = 5; i < 10; i++) level[17][i] = '#';
        
        // Walls
        for (int i = 0; i < height; i++) {
            level[i][0] = '#';
            level[i][width-1] = '#';
        }
        
        // Player start - on the ground
        player.x = 5;
        player.y = height - 2;
        player.vy = 0;  
        player.grounded = true;
    }

    bool isBlocked(int x, int y) {
        if (y < 0 || y >= height || x < 0 || x >= width) return true;
        return level[y][x] == '#';
    }

    void input() {
        if (_kbhit()) {
            char key = _getch();
            
            // Arrow keys
            if (key == -32 || key == 0) {
                key = _getch();
                if (key == 75) { // Left
                    if (!isBlocked(player.x - 1, player.y)) {
                        player.x--;
                    }
                }
                if (key == 77) { // Right
                    if (!isBlocked(player.x + 1, player.y)) {
                        player.x++;
                    }
                }
            }
            
            // Space to jump
            if (key == 72) {
                if (player.grounded) {
                    player.vy = JUMP;
                    player.grounded = false;
                }
            }
            
            // ESC to exit
            if (key == 27) {
                exit(0);
            }
        }
    }

    void physics() {
        // Only apply gravity if not grounded
        if (!player.grounded) {
            player.vy += GRAVITY;
            if (player.vy > MAX_FALL) player.vy = MAX_FALL;
        }
        
        // Apply vertical velocity
        if (player.vy != 0) {
            int steps = (int)(fabs(player.vy) + 0.5);
            int dir = (player.vy > 0) ? 1 : -1;
            
            for (int i = 0; i < steps; i++) {
                int newY = player.y + dir;
                
                if (isBlocked(player.x, newY)) {
                    player.vy = 0;
                    if (dir > 0) {
                        player.grounded = true;
                    }
                    break;
                } else {
                    player.y = newY;
                }
            }
        }
        
        // Check if standing on ground
        if (isBlocked(player.x, player.y + 1)) {
            player.grounded = true;
            player.vy = 0;
        } else {
            player.grounded = false;
        }
    }

    void goalReached(){
        if(player.x == 15 && player.y == 4){
            cout << "\n\nLEVEL COMPLETE";
            exit(0);
        }
    }

    void render() {
        // Move cursor to top-left
        COORD pos = {0, 0};
        SetConsoleCursorPosition(hConsole, pos);
        
        // Build output string
        string output;
        output.reserve((width + 3) * (height + 4));
        
        // Top border
        for (int x = 0; x < width+2; x++) output += '=';
        output += '\n';
        
        // Level
        for (int y = 0; y < height; y++) {
            output += '|';
            for (int x = 0; x < width; x++) {
                if (x == player.x && y == player.y) {
                    output += '@';
                } else if (level[y][x] == '#') {
                    output += '#';
                } else {
                    output += ' ';
                }
            }
            output += "|\n";
        }
        
        
        // Controls
        output += " Arrow Keys = Move | ESC = Exit";
        
        // Write all at once
        DWORD written;
        WriteConsole(hConsole, output.c_str(), output.length(), &written, NULL);
    }

    void run() {
        system("cls");
        
        while (true) {
            input();
            physics();
            render();
            goalReached();
            Sleep(50);
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}