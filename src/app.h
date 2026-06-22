#pragma once

#include "renderer.h"
#include "particleSystem.h"

struct GLFWwindow;

// Top-level application: owns the window and drives the main loop.
class App {
public:
    App(int width, int height, const char* title);
    ~App();

    void run();

private:
    GLFWwindow* m_window = nullptr;
    Renderer m_renderer;
    ParticleSystem m_system;
    int m_width = 0;
    int m_height = 0;
};