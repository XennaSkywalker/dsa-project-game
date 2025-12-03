#include <iostream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <cmath>
#include "SaveManager.h"
#include "ReplayManager.h"

using namespace std;

// ---------------------------------------------------------
// PLAYER STRUCT
// ---------------------------------------------------------
struct Player {
    int x, y;
    double vy;
    bool grounded;
};

// ---------------------------------------------------------
// LEVEL CLASS
// ---------------------------------------------------------
class Level {
private:
    int width, height;
    vector<vector<char>> grid;

public:
    int goalX, goalY;

    Level(int w, int h) : width(w), height(h) {
        grid = vector<vector<char>>(height, vector<char>(width, ' '));
    }

    void createPlatform(int y, int startX, int length) {
        for (int x = startX; x < startX + length && x < width; x++)
            grid[y][x] = '#';
    }

    void createFullGround() {
        for (int x = 0; x < width; x++)
            grid[height - 1][x] = '#';
    }

    void createWalls() {
        for (int y = 0; y < height; y++) {
            grid[y][0] = '#';
            grid[y][width - 1] = '#';
        }
    }

    void setGoal(int x, int y) {
        goalX = x;
        goalY = y;
    }

    bool isBlocked(int x, int y) {
        if (x < 0 || x >= width || y < 0 || y >= height) return true;
        return grid[y][x] == '#';
    }

    char getTile(int x, int y) {
        return grid[y][x];
    }
};

// ---------------------------------------------------------
// GAME CLASS
// ---------------------------------------------------------
class Game {
private:
    HANDLE hConsole;
    int width = 60;
    int height = 20;

    Player player;
    SaveManager saveManager;
    ReplayManager replayManager;
    vector<Level*> levels;
    int currentLevelIndex = 0;

    const double GRAVITY = 0.3;
    const double JUMP = -2.0;
    const double MAX_FALL = 2.0;

public:
    Game() {
        setupWindow();
        loadLevels();
        loadLevel(0);
    }

    ~Game() {
        for (auto* lvl : levels) delete lvl;
    }

    void setupWindow() {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        COORD bufferSize = {(SHORT)(width + 3), (SHORT)(height + 4)};
        SetConsoleScreenBufferSize(hConsole, bufferSize);

        SMALL_RECT windowSize = {0, 0, (SHORT)(width + 2), (SHORT)(height + 3)};
        SetConsoleWindowInfo(hConsole, TRUE, &windowSize);

        CONSOLE_CURSOR_INFO ci;
        GetConsoleCursorInfo(hConsole, &ci);
        ci.bVisible = false;
        SetConsoleCursorInfo(hConsole, &ci);

        SetConsoleTitle("OOP Platformer Game with Replay");
    }

    void loadLevels() {
        // LEVEL 1
        Level* lvl1 = new Level(width, height);
        lvl1->createFullGround();
        lvl1->createWalls();
        lvl1->createPlatform(14, 10, 10);
        lvl1->createPlatform(11, 23, 12);
        lvl1->createPlatform(8,  37, 13);
        lvl1->createPlatform(7,  29, 4);
        lvl1->createPlatform(5,  15, 10);
        lvl1->createPlatform(17, 5, 5);
        lvl1->setGoal(15, 4);
        levels.push_back(lvl1);

        // LEVEL 2
        Level* lvl2 = new Level(width, height);
        lvl2->createFullGround();
        lvl2->createWalls();
        lvl2->createPlatform(16,  8, 12);
        lvl2->createPlatform(12, 20, 10);
        lvl2->createPlatform(9,  32, 15);
        lvl2->createPlatform(6,  45, 10);
        lvl2->setGoal(50, 5);
        levels.push_back(lvl2);
    }

    void loadLevel(int index) {
        currentLevelIndex = index;
        player.x = 5;
        player.y = height - 2;
        player.vy = 0;
        player.grounded = true;

        saveManager.clear();
        replayManager.clear(); // Clear replay queue on new level
        system("cls");
    }

    Level* currentLevel() {
        return levels[currentLevelIndex];
    }

    void input() {
        if (!_kbhit()) return;
        char key = _getch();

        if (!replayManager.isReplaying()) {
            // Arrow keys
            if (key == -32 || key == 0) {
                key = _getch();
                if (key == 75) { // LEFT
                    if (!currentLevel()->isBlocked(player.x - 1, player.y))
                        player.x--;
                }
                if (key == 77) { // RIGHT
                    if (!currentLevel()->isBlocked(player.x + 1, player.y))
                        player.x++;
                }
            }

            if (key == 72 && player.grounded) { // UP arrow → jump
                player.vy = JUMP;
                player.grounded = false;
            }

            // Save player state
            if (key == 's' || key == 'S') {
                PlayerState state = {player.x, player.y, player.vy, player.grounded};
                saveManager.saveState(state);
                cout << "\nPlayer state saved! (" << player.x << "," << player.y << ")\n";
            }

            // Undo last saved state
            if (key == 'u' || key == 'U') {
                PlayerState state;
                if (saveManager.undoState(state)) {
                    player.x = state.x;
                    player.y = state.y;
                    player.vy = state.vy;
                    player.grounded = state.grounded;
                    cout << "\nReverted to last saved state: (" << player.x << "," << player.y << ")\n";
                } else {
                    cout << "\nNo saved state to undo!\n";
                }
            }

            // Start replay
            if (key == 'r' || key == 'R') {
                replayManager.startReplay();
                cout << "\nReplay started!\n";
            }
        }

        if (key == 27) // ESC
            exit(0);
    }

    void physics() {
        Level* lvl = currentLevel();

        if (!player.grounded) {
            player.vy += GRAVITY;
            if (player.vy > MAX_FALL) player.vy = MAX_FALL;
        }

        if (player.vy != 0) {
            int steps = int(fabs(player.vy) + 0.5);
            int dir = (player.vy > 0) ? 1 : -1;

            for (int i = 0; i < steps; i++) {
                int newY = player.y + dir;
                if (lvl->isBlocked(player.x, newY)) {
                    player.vy = 0;
                    if (dir > 0) player.grounded = true;
                    break;
                } else {
                    player.y = newY;
                }
            }
        }

        if (lvl->isBlocked(player.x, player.y + 1)) {
            player.grounded = true;
            player.vy = 0;
        } else {
            player.grounded = false;
        }
    }

    void checkGoal() {
        Level* lvl = currentLevel();

        if (player.x == lvl->goalX && player.y == lvl->goalY) {
            if (currentLevelIndex + 1 < levels.size()) {
                cout << "\n\nLEVEL COMPLETE! Loading next level...\n";
                Sleep(1000);
                loadLevel(currentLevelIndex + 1);
            } else {
                cout << "\n\nYOU FINISHED ALL LEVELS!\n";
                exit(0);
            }
        }
    }

    void render() {
        Level* lvl = currentLevel();

        COORD pos = {0, 0};
        SetConsoleCursorPosition(hConsole, pos);

        string out;
        out.reserve(8000);

        // Top border
        for (int x = 0; x < width + 2; x++) out += '=';
        out += '\n';

        for (int y = 0; y < height; y++) {
            out += '|';
            for (int x = 0; x < width; x++) {
                if (x == player.x && y == player.y)
                    out += '@';
                else if (x == lvl->goalX && y == lvl->goalY)
                    out += 'G';
                else
                    out += lvl->getTile(x, y);
            }
            out += "|\n";
        }

        out += " Arrow Keys = Move | Jump = UP ARROW | S = Save | U = Undo | R = Replay | ESC = EXIT";

        DWORD written;
        WriteConsole(hConsole, out.c_str(), out.length(), &written, NULL);
    }

    void run() {
        while (true) {
            input();

            if (replayManager.isReplaying()) {
                PlayerState state;
                if (replayManager.getNext(state)) {
                    player.x = state.x;
                    player.y = state.y;
                    player.vy = state.vy;
                    player.grounded = state.grounded;
                }
            } else {
                physics();
                // Record player moves
                PlayerState current = {player.x, player.y, player.vy, player.grounded};
                replayManager.recordMove(current);
            }

            render();
            checkGoal();
            Sleep(50);
        }
    }
};

// ---------------------------------------------------------
// MAIN
// ---------------------------------------------------------
int main() {
    Game game;
    game.run();
    return 0;
}
