#include "app.h"


App::App(int width, int height, const char* title)
    : m_width(width), m_height(height), title(title) {
    bool initWindowSucess = window.init(m_width, m_height, title);
    
    if (!initWindowSucess) std::cout << "error setting up window" << std::endl;
    initImGui();
    switchScene(SceneId::CubeTest);
}

void App::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window.window, true);
    ImGui_ImplOpenGL3_Init("#version 430");
}

App::~App() {
    // TODO: tear down GL resources and the window.
}

void App::drawCommonUI() {
    const char* title = m_currentScene->m_title.c_str();
    ImGui::Begin(title);
    const char* sceneNames[] = {"CubeScene", "FluidSim", "TerrainScene"};
    if (ImGui::Combo("Scene", &m_sceneIndex, sceneNames, IM_ARRAYSIZE(sceneNames))) {
        switchScene(static_cast<SceneId>(m_sceneIndex));
    }
    ImGui::End();
}



void App::run() {
    float deltaTime = 0;
    float previousTime = glfwGetTime();
    while (!glfwWindowShouldClose(window.window)) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        handleInputs(window.window);
        float time = glfwGetTime();
        deltaTime = time - previousTime;

        m_currentScene->update(deltaTime,fInput);

        drawCommonUI();
        m_currentScene->drawUI();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window.window);
        glfwPollEvents();
        previousTime = time;
    }
    m_currentScene->onExit();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

void App::handleInputs(GLFWwindow* window) {
    fInput.setKeyState(Key::CloseWindow, glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS);
    fInput.setKeyState(Key::RotateRight, glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS);
    fInput.setKeyState(Key::RotateLeft, glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS);
    fInput.setKeyState(Key::ToggleMouse, glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);

    processInput(window);
    if (!mouseToggle) {
        getMouseInput(window);
    }
    fInput.updateKeyState();
}

void App::getMouseInput(GLFWwindow* window) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    if (fInput.firstMouse) {
        fInput.lastX = x;
        fInput.lastY = y;
        fInput.firstMouse = false;
    }
    double dx = x - fInput.lastX;
    double dy = fInput.lastY - y;
    fInput.dx = dx;
    fInput.dy = dy;
    fInput.lastX = x; fInput.lastY = y;
}

void App::processInput(GLFWwindow* window) {
    if (fInput.isDown(Key::CloseWindow)) {
        glfwSetWindowShouldClose(window, true);
    }
    if (fInput.isJustPressed(Key::ToggleMouse)) {
        if (!mouseToggle) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            fInput.dx = 0.0;
            fInput.dy = 0.0;
            mouseToggle = !mouseToggle;
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            mouseToggle = !mouseToggle;
        }
    }
}

void App::switchScene(SceneId id) {
    if (m_currentScene) {
        m_currentScene->onExit();
    }

    switch (id) {
        case SceneId::CubeTest: m_currentScene = std::make_unique<CubeScene>(m_width, m_height, "CubeScene"); break;
        case SceneId::TerrainScene: m_currentScene = std::make_unique<TerrainScene>(m_width, m_height, "TerrainScene"); break;
        default: 
            std::cout << "switchScene: unhandled Sceneid" << std::endl;
            return;
    }

    m_currentScene->onEnter();
}
