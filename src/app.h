#pragma once

#include "FluidSim.h"
#include "keyHandler.h"
#include "Window.h"
#include <iostream>
#include "Scene.h"
#include "CubeScene.h"
#include "TerrainScene.h"
#include <memory>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


enum class SceneId{CubeTest, FluidSim, TerrainScene};


// Top-level application: owns the window and drives the main loop.
class App {
public:
    App(int width, int height, const char* title);
    ~App();

    void initImGui();
    void drawCommonUI();
    void run();
    void handleInputs(GLFWwindow* window);
    void getMouseInput(GLFWwindow* window);
    void processInput(GLFWwindow* window);
    void switchScene(SceneId id);

private:
    Window window;
    std::unique_ptr<Scene> m_currentScene;
    std::string title;
    int m_width = 0;
    int m_height = 0;
    int m_sceneIndex = 1;
    frameInput fInput;
    bool mouseToggle = true;
};