#pragma once
#include <string>
#include <vector>
#include <map>

// Tree Node
struct DecisionNode {
    int id;                         // Unique ID for the decision
    std::string description;        // Text describing the option
    int targetLevelID;              // Which level this option loads
    std::vector<DecisionNode*> children; // Possible next choices (if any)

    DecisionNode(int _id, std::string desc, int _level) 
        : id(_id), description(desc), targetLevelID(_level) {}
};

class DecisionTree {
public:
    DecisionNode* root;
    
    DecisionTree() {
        // Root is the decision point (e.g., standing in front of doors)
        root = new DecisionNode(0, "Choose your path", 0);

        // Child 1: Left Door -> Forest
        root->children.push_back(new DecisionNode(1, "Enter the Green Door (Forest)", 2));

        // Child 2: Right Door -> Cave
        root->children.push_back(new DecisionNode(2, "Enter the Stone Door (Cave)", 3));
    }

    // Helper to get options for the frontend
    std::vector<std::pair<int, std::string>> getOptions() {
        std::vector<std::pair<int, std::string>> options;
        if (root) {
            for (auto child : root->children) {
                options.push_back({child->id, child->description});
            }
        }
        return options;
    }

    // Find which level to load based on choice ID
    int getTargetLevel(int choiceId) {
        if (!root) return -1;
        for (auto child : root->children) {
            if (child->id == choiceId) return child->targetLevelID;
        }
        return -1;
    }
};