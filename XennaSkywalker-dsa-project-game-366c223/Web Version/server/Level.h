#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include "DecisionTree.h" // Include the tree

struct Door {
    int x, y;
    bool active;
};

class Level {
private:
    int width, height;
    std::vector<std::vector<char>> grid;

public:
    int goalX, goalY;
    std::vector<Door> doors; // List of doors in this level

    Level(int w, int h) : width(w), height(h) {
        resetGrid();
    }

    void resetGrid() {
        grid = std::vector<std::vector<char>>(height, std::vector<char>(width, ' '));
        doors.clear();
    }

    // Place a horizontal platform
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

    // New: Add a door to the level
    void addDoor(int x, int y) {
        doors.push_back({x, y, true});
        if (y >= 0 && y < height && x >= 0 && x < width)
            grid[y][x] = 'D'; // Visual representation of a Door
    }

    bool isBlocked(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return true;
        return grid[y][x] == '#';
    }

    char getTile(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return '#';
        return grid[y][x];
    }
    
    // Check if player is standing on a door
    bool isDoor(int x, int y) const {
        for(const auto& d : doors) {
            if(d.x == x && d.y == y) return true;
        }
        return false;
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

#endif // LEVEL_H