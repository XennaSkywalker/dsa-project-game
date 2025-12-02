#pragma once
#include <queue>
#include "SaveManager.h"  // For PlayerState

class ReplayManager {
private:
    std::queue<PlayerState> replayQueue;

    bool replaying = false;

public:
    // Record player moves during gameplay
    void recordMove(const PlayerState& state) {
        if (!replaying) replayQueue.push(state);
    }

    // Start replay
    void startReplay() {
        replaying = true;
    }
    
    // Stop replay
    void stopReplay() {
        replaying = false;
    }
    
    // Check if replay is active
    bool isReplaying() const {
        return replaying;
    }
    
    // Get next move in replay
    bool getNext(PlayerState& state) {
        if (!replaying || replayQueue.empty()) return false;
        state = replayQueue.front();
        replayQueue.pop();
        if (replayQueue.empty()) replaying = false;
        return true;
    }

    // Clear all recorded moves
    void clear() {
        std::queue<PlayerState> empty;
        std::swap(replayQueue, empty);
        replaying = false;
    }
};
