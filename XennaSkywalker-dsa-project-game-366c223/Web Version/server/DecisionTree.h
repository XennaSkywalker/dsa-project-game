#pragma once
#include <string>
#include <vector>

struct DecisionNode {
    int id;
    std::string description;
    int targetLevelID;
    std::vector<DecisionNode*> children;

    DecisionNode(int _id, std::string desc, int _level) 
        : id(_id), description(desc), targetLevelID(_level) {}

    // RECURSIVE DESTRUCTOR: Deletes all children automatically
    ~DecisionNode() {
        for (auto child : children) {
            delete child;
        }
    }
};

class DecisionTree {
public:
    DecisionNode* root;
    
    DecisionTree() {
        root = new DecisionNode(0, "Choose your path", 0);
        root->children.push_back(new DecisionNode(1, "Enter the Green Door (Forest)", 2));
        root->children.push_back(new DecisionNode(2, "Enter the Stone Door (Cave)", 3));
    }

    ~DecisionTree() {
        delete root; // Triggers the recursive deletion above
    }

    std::vector<std::pair<int, std::string>> getOptions() {
        std::vector<std::pair<int, std::string>> options;
        if (root) {
            for (auto child : root->children) {
                options.push_back({child->id, child->description});
            }
        }
        return options;
    }

    int getTargetLevel(int choiceId) {
        if (!root) return -1;
        for (auto child : root->children) {
            if (child->id == choiceId) return child->targetLevelID;
        }
        return -1;
    }
};