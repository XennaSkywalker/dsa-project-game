#pragma once
#include <queue>
#include "GameState.h"

class ReplayManager {
    std::queue<GameState> replayQueue;
public:
    void record(const GameState& state) {
        replayQueue.push(state);
    }

    void clear() {
        while (!(replayQueue.empty()))
        replayQueue.pop();
    }

    bool next(GameState& state) {
        if (replayQueue.empty()) return false;
        state = replayQueue.front();
        replayQueue.pop();
        return true;
    }

    std::queue<GameState> copy() {
        return replayQueue;
    }
};
