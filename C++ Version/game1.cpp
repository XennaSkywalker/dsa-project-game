#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <cmath>

using namespace std;

class Player {
public:
    int x, y;
    double vy;
    bool grounded;

    Player() : x(0), y(0), vy(0), grounded(false) {}

    void setStart(int sx, int sy) {
        x = sx;
        y = sy;
        vy = 0;
        grounded = true;
    }
};

class Level {
private:
    vector<vector<char>> grid;

public:
    int width, height;
    int goalX, goalY;

    Level(int w, int h) : width(w), height(h) {
        grid = vector<vector<char>>(height, vector<char>(width, ' '));
        goalX = goalY = -1;
    }

    void clear() {
        for (auto &row : grid) fill(row.begin(), row.end(), ' ');
    }

    void createGround() {
        for (int i = 0; i < width; i++) grid[height - 1][i] = '#';
    }

    void createWalls() {
        for (int i = 0; i < height; i++) {
            grid[i][0] = '#';
            grid[i][width - 1] = '#';
        }
    }

    // Create a horizontal platform
    void createPlatform(int y, int xStart, int xEnd) {
        for (int x = xStart; x <= xEnd && x < width; x++) {
            grid[y][x] = '#';
        }
    }

    void setGoal(int gx, int gy) {
        goalX = gx;
        goalY = gy;
    }

    bool isBlocked(int x, int y) {
        if (y < 0 || y >= height || x < 0 || x >= width) return true;
        return grid[y][x] == '#';
    }

    char getTile(int x, int y) {
        if (x == goalX && y == goalY) return 'G'; // goal
        return grid[y][x];
    }
};

class Game {
private:
    Player player;
    Level* currentLevel;
    HANDLE hConsole;

    const double GRAVITY = 0.4;
    const double JUMP = -2.0;
    const double MAX_FALL = 2.0;

public:
    Game() {
        // Fixed window size
        int width = 60, height = 20;

        currentLevel = new Level(width, height);
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        setupConsoleWindow(width, height);
        loadLevel1();
    }

    ~Game() {
        delete currentLevel;
    }

    void setupConsoleWindow(int width, int height) {
        COORD bufferSize = {(SHORT)(width + 3), (SHORT)(height + 4)};
        SetConsoleScreenBufferSize(hConsole, bufferSize);

        SMALL_RECT windowSize = {0, 0, (SHORT)(width + 2), (SHORT)(height + 3)};
        SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = false;
        SetConsoleCursorInfo(hConsole, &cursorInfo);
    }

//level 1
    void loadLevel1() {
        Level &L = *currentLevel;

        L.clear();
        L.createGround();
        L.createWalls();

        L.createPlatform(14, 10, 20);
        L.createPlatform(11, 23, 35);
        L.createPlatform(8, 37, 50);
        L.createPlatform(7, 29, 33);
        L.createPlatform(5, 15, 25);
        L.createPlatform(17, 5, 10);

        player.setStart(5, L.height - 2);
        L.setGoal(15, 4);
    }

    void loadLevel2() {
        Level &L = *currentLevel;

        L.clear();
        L.createGround();
        L.createWalls();

        L.createPlatform(13, 10, 20);
        L.createPlatform(11, 23, 35);
        L.createPlatform(5, 37, 50);
        L.createPlatform(7, 29, 33);
        L.createPlatform(17, 5, 10);

        player.setStart(5, L.height - 2);
        L.setGoal(15, 4);
    }

    void input() {
        if (_kbhit()) {
            char key = _getch();

            if (key == -32 || key == 0) {
                key = _getch();
                if (key == 75) { // left
                    if (!currentLevel->isBlocked(player.x - 1, player.y))
                        player.x--;
                }
                if (key == 77) { // right
                    if (!currentLevel->isBlocked(player.x + 1, player.y))
                        player.x++;
                }
            }

            // Jump ↑
            if (key == 72 && player.grounded) {
                player.vy = JUMP;
                player.grounded = false;
            }

            if (key == 27) exit(0);
        }
    }

    void physics() {
        if (!player.grounded) {
            player.vy += GRAVITY;
            if (player.vy > MAX_FALL) player.vy = MAX_FALL;
        }

        if (player.vy != 0) {
            int steps = (int)(fabs(player.vy) + 0.5);
            int dir = (player.vy > 0) ? 1 : -1;

            for (int i = 0; i < steps; i++) {
                int newY = player.y + dir;

                if (currentLevel->isBlocked(player.x, newY)) {
                    player.vy = 0;
                    if (dir > 0) player.grounded = true;
                    break;
                } else {
                    player.y = newY;
                }
            }
        }

        // Standing check
        if (currentLevel->isBlocked(player.x, player.y + 1)) {
            player.grounded = true;
            player.vy = 0;
        } else {
            player.grounded = false;
        }
    }

    void goalCheck() {
        if (player.x == currentLevel->goalX &&
            player.y == currentLevel->goalY) 
        {
            cout << "\n\n   LEVEL COMPLETE!";
            exit(0);
        }
    }

    void render() {
        COORD pos = {0, 0};
        SetConsoleCursorPosition(hConsole, pos);

        Level &L = *currentLevel;

        string out;
        out.reserve((L.width + 3) * (L.height + 4));

        // Top border
        for (int i = 0; i < L.width + 2; i++) out += '=';
        out += '\n';

        for (int y = 0; y < L.height; y++) {
            out += '|';
            for (int x = 0; x < L.width; x++) {
                if (x == player.x && y == player.y)
                    out += '@';
                else
                    out += L.getTile(x, y);
            }
            out += "|\n";
        }

        out += " Arrow Keys = Move | Space = Jump | ESC = Exit";

        DWORD written;
        WriteConsole(hConsole, out.c_str(), out.size(), &written, NULL);
    }

    // ------------------------------------------
    // MAIN LOOP
    // ------------------------------------------
    void run() {
        system("cls");
        while (true) {
            input();
            physics();
            render();
            goalCheck();
            Sleep(50);
        }
    }
};


int main() {
    Game game;
    game.run();
    return 0;
}
