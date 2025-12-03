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
        if((state.player.x == saves.top().player.x) && (state.player.y == saves.top().player.y)) saves.pop();
        if (saves.empty()) saves.push(state);
        state = saves.top();
        return true;
    }
};
