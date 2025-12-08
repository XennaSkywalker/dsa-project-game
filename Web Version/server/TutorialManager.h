#pragma once
#include <string>

struct TutorialStep {
    std::string message;
    std::string requiredInput;
    TutorialStep* next;

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
        addStep("Welcome! Press 'right' to move.", "right");
        addStep("Great! Now press 'left' to move back.", "left");
        addStep("Press 'up' to jump!", "up");
        current = head;
    }

    // DESTRUCTOR: Cleans up memory when the game closes
    ~TutorialManager() {
        TutorialStep* temp = head;
        while (temp) {
            TutorialStep* nextNode = temp->next;
            delete temp;
            temp = nextNode;
        }
    }

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

    bool checkProgress(const std::string& inputKey) {
        if (!isActive || !current) return true; 
        if (inputKey == current->requiredInput) {
            current = current->next;
            if (!current) isActive = false; 
            return true;
        }
        return false;
    }

    std::string getCurrentMessage() {
        if (isActive && current) return current->message;
        return "";
    }
};