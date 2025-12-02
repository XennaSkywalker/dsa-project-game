#pragma once
#include <queue>
#include "GameState.h"

class ReplayManager {
    std::queue<GameState> replayQueue;
public:
    void record(const GameState& state) {
        replayQueue.push(state);
    }

    bool next(GameState& state) {
        if (replayQueue.empty()) return false;
        state = replayQueue.front();
        replayQueue.pop();
        return true;
    }

    void reset(std::queue<GameState> q) {
        replayQueue = q; // copy for replay
    }

    std::queue<GameState> copy() {
        return replayQueue;
    }
};
