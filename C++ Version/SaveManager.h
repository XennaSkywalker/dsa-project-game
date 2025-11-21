#pragma once
#include <stack>

// Struct to store entire player state
struct PlayerState {
    int x, y;
    double vy;
    bool grounded;
};

class SaveManager {
private:
    std::stack<PlayerState> savedStates;

public:
    // Save the current player state
    void saveState(const PlayerState& state) {
        savedStates.push(state);
    }

    // Undo the last saved state, returns false if no saved state
    bool undoState(PlayerState &state) {
        if (savedStates.empty()) return false;
        state = savedStates.top();
        savedStates.pop();
        return true;
    }

    // Check if any saved state exists
    bool hasSavedState() const {
        return !savedStates.empty();
    }

    // Clear all saved states
    void clear() {
        while (!savedStates.empty()) savedStates.pop();
    }
};
