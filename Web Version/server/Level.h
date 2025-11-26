#ifndef LEVEL_H
#define LEVEL_H

#include <vector>

class Level {
private:
    int width, height;
    std::vector<std::vector<char>> grid;

public:
    int goalX, goalY;

    Level(int w, int h) : width(w), height(h) {
        grid = std::vector<std::vector<char>>(height, std::vector<char>(width, ' '));
    }

    // Place a horizontal platform
    void createPlatform(int y, int startX, int length) {
        for (int x = startX; x < startX + length && x < width; x++)
            grid[y][x] = '#';
    }

    // Fill the bottom with ground
    // void createFullGround() {
    //     for (int x = 0; x < width; x++)
    //         grid[height - 1][x] = '!';
    // }

    // Add walls on the sides
    // void createWalls() {
    //     for (int y = 0; y < height; y++) {
    //         grid[y][0] = '!';
    //         grid[y][width - 1] = '!';
    //     }
    // }

    void setGoal(int x, int y) {
        goalX = x;
        goalY = y;
        if (y >= 0 && y < height && x >= 0 && x < width)
            grid[y][x] = 'G';
    }

    bool isBlocked(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return true;
        return grid[y][x] == '#';
    }

    char getTile(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return '#';
        return grid[y][x];
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

#endif // LEVEL_H
