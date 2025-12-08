#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include "DecisionTree.h"

struct Door {
    int x, y;
    bool active;
};

class Level {
private:
    int width, height;
    std::vector<std::vector<char>> grid;

public:
    int goalX = -1, goalY = -1;
    std::vector<Door> doors;

    Level(int w, int h) : width(w), height(h) {
        resetGrid();
    }

    void resetGrid() {
        grid = std::vector<std::vector<char>>(height, std::vector<char>(width, ' '));
        doors.clear();
    }

    void createPlatform(int y, int startX, int length) {
        for (int x = startX; x < startX + length && x < width; x++)
            grid[y][x] = '#';
    }

    void setGoal(int x, int y) {
        goalX = x;
        goalY = y;
        if (y >= 0 && y < height && x >= 0 && x < width)
            grid[y][x] = 'G';
    }

    void addDoor(int x, int y) {
        doors.push_back({x, y, true});
        if (y >= 0 && y < height && x >= 0 && x < width)
            grid[y][x] = 'D';
    }

    bool isBlocked(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return true;
        return grid[y][x] == '#';
    }

    char getTile(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return '#';
        return grid[y][x];
    }
    
    bool isDoor(int x, int y) const {
        for(const auto& d : doors) {
            if(d.x == x && d.y == y) return true;
        }
        return false;
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

#endif