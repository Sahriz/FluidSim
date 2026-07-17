#include "app.h"


App::App(int width, int height, const char* title)
    : m_width(width), m_height(height), title(title) {
    bool initWindowSucess = window.init(m_width, m_height, title);
    
    if (!initWindowSucess) std::cout << "error setting up window" << std::endl;
    initImGui();
    switchScene(SceneId::FluidSim);
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
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const float panelWidth = 240.0f;
    ImGui::SetNextWindowPos(vp->WorkPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, vp->WorkSize.y), ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    const char* title = m_currentScene->m_title.c_str();
    ImGui::Begin(title, nullptr, flags);
    ImGui::PushItemWidth(-140.0f);
    float fps = ImGui::GetIO().Framerate;
    ImGui::Text("%.1f FPS (%.2f ms/frame)", fps, 1000.0f / fps);
    const char* sceneNames[] = {"CubeScene", "FluidSim", "TerrainScene"};
    if (ImGui::Combo("Scene", &m_sceneIndex, sceneNames, IM_ARRAYSIZE(sceneNames))) {
        switchScene(static_cast<SceneId>(m_sceneIndex));
    }
    ImGui::Separator();
    m_currentScene->drawUI();
    ImGui::PopItemWidth();
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
        m_currentScene->render();
        
        drawCommonUI();
        

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
    fInput.setKeyState(Key::MoveLeft, glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
    fInput.setKeyState(Key::MoveRight, glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
    fInput.setKeyState(Key::MoveForward, glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    fInput.setKeyState(Key::MoveBackward, glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    fInput.setKeyState(Key::Descend, glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
    fInput.setKeyState(Key::Ascend, glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);

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
    fInput.inputDirection = glm::vec3(0.0);
    if (fInput.isDown(Key::MoveLeft)) {
        fInput.inputDirection.x += -1;
    }
    if (fInput.isDown(Key::MoveRight)) {
        fInput.inputDirection.x += 1;
    }
    if (fInput.isDown(Key::MoveForward)) {
        fInput.inputDirection.z += 1;
    }
    if (fInput.isDown(Key::MoveBackward)) {
        fInput.inputDirection.z += -1;
    }
    if (fInput.isDown(Key::Descend)) {
        fInput.inputDirection.y += -1;
    }
    if (fInput.isDown(Key::Ascend)) {
        fInput.inputDirection.y += 1;
    }
}

void App::switchScene(SceneId id) {
    if (m_currentScene) {
        m_currentScene->onExit();
    }

    switch (id) {
        case SceneId::CubeTest: m_currentScene = std::make_unique<CubeScene>(m_width, m_height, "CubeScene"); break;
        case SceneId::FluidSim: m_currentScene = std::make_unique<FluidSim>(m_width, m_height, "FluidSim"); break;
        case SceneId::TerrainScene: m_currentScene = std::make_unique<TerrainScene>(m_width, m_height, "TerrainScene"); break;
        default: 
            std::cout << "switchScene: unhandled Sceneid" << std::endl;
            return;
    }

    m_currentScene->onEnter();
}
