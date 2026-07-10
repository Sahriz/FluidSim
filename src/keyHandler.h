#pragma once
#include <array>
#include "glm.hpp"

enum class Key {
    RotateLeft,
    RotateRight,
    CloseWindow,
    ToggleMouse,
    MoveLeft,
    MoveRight,
    MoveForward,
    MoveBackward,
    Descend,
    Ascend,
    Count
};

struct frameInput {
public:
    std::array<bool, static_cast<size_t>(Key::Count)> currentKeys{ false };
    std::array<bool, static_cast<size_t>(Key::Count)> previousKeys{ false };
    double dx = 0.0, dy = 0.0;
    double lastX = 0.0, lastY = 0.0;
    bool firstMouse = true;
    glm::vec3 inputDirection = glm::vec3(0.0);

    void updateKeyState() {
        previousKeys = currentKeys;
    }

    void setKeyState(Key key, bool isDown) {
        currentKeys[static_cast<size_t>(key)] = isDown;
    }

    bool isDown(Key key) const{
        return currentKeys[static_cast<size_t>(key)];
    }

    bool isJustPressed(Key key) const {
        size_t idx = static_cast<size_t>(key);
        return currentKeys[idx] && !previousKeys[idx];
    }

    bool isJustReleased(Key key) const {
        size_t idx = static_cast<size_t>(key);
        return !currentKeys[idx] && previousKeys[idx];
    }
};
