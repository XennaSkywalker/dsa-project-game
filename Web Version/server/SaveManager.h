#pragma once
#include <stack>
#include "GameState.h"

class SaveManager {
    std::stack<GameState> saves;
public:
    void save(const GameState& state) {
        saves.push(state);
    }

    bool undo(GameState& state) {
        if (saves.empty()) return false;
        state = saves.top();
        saves.pop();
        return true;
    }
};
