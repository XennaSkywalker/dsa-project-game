#pragma once
#include <string>
#include <iostream>

// Node structure for Linked List
struct TutorialStep {
    std::string message;        // Text to show the user
    std::string requiredInput;  // Input key required to advance (e.g., "right")
    TutorialStep* next;         // Pointer to the next step

    TutorialStep(std::string msg, std::string req) 
        : message(msg), requiredInput(req), next(nullptr) {}
};

class TutorialManager {
private:
    TutorialStep* head;
    TutorialStep* current;

public:
    bool isActive;

    TutorialManager() : head(nullptr), current(nullptr), isActive(true) {
        // Build the Linked List:
        // Step 1 -> Step 2 -> Step 3 -> nullptr
        addStep("Welcome! Press 'right' to move.", "right");
        addStep("Great! Now press 'left' to move back.", "left");
        addStep("Press 'up' to jump!", "up");
        
        current = head;
    }

    // Helper to add nodes to the end of the list
    void addStep(std::string msg, std::string req) {
        TutorialStep* newStep = new TutorialStep(msg, req);
        if (!head) {
            head = newStep;
            current = head;
        } else {
            TutorialStep* temp = head;
            while (temp->next) temp = temp->next;
            temp->next = newStep;
        }
    }

    // Check input and traverse the list
    bool checkProgress(const std::string& inputKey) {
        if (!isActive || !current) return true; // Tutorial finished or inactive

        if (inputKey == current->requiredInput) {
            current = current->next; // Move to next node
            if (!current) {
                isActive = false; // End of list
                return true;
            }
            return true; // Input was correct
        }
        return false; // Input was incorrect for this step
    }

    std::string getCurrentMessage() {
        if (isActive && current) return current->message;
        return "";
    }
};