#pragma once

#include "particleSystem.h"

// Draws the scene. Owns shader/program/VAO state.
class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init(int width, int height);
    void render(ParticleSystem& system);
    void cleanup();
};