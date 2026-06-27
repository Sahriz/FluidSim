#pragma once

#include "renderer.h"
#include "FluidSim.h"
#include "keyHandler.h"
#include <iostream>


// Top-level application: owns the window and drives the main loop.
class App {
public:
    App(int width, int height, const char* title);
    ~App();

    void run();
    void handleInputs(GLFWwindow* window);
    void getMouseInput(GLFWwindow* window);
    void processInput(GLFWwindow* window);

private:
    Renderer m_renderer;
    FluidSim m_system;
    std::string title;
    int m_width = 0;
    int m_height = 0;
    frameInput fInput;
    bool mouseToggle = true;
};