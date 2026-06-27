#include "app.h"


App::App(int width, int height, const char* title)
    : m_width(width), m_height(height), title(title) {
    bool initSuccess = m_renderer.init(width, height, title);
}

App::~App() {
    // TODO: tear down GL resources and the window.
}



void App::run() {
    GLFWwindow* window = m_renderer.GetWindow();
    while (!glfwWindowShouldClose(window)) {
        handleInputs(window);

        m_renderer.render(m_system, fInput);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            mouseToggle = !mouseToggle;
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            fInput.dx = 0.0;
            fInput.dy = 0.0;
            mouseToggle = !mouseToggle;
        }
    }
}
