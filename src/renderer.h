#pragma once

#include "FluidSim.h"
#include <iostream>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Shader.h"
#include "Camera.h"
#include "keyHandler.h"


// Draws the scene. Owns shader/program/VAO state.
class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init(int width, int height, const char* title);
    void render(FluidSim& system, frameInput& fInput);
    void cleanup();

    void tempCode();

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* GetWindow() {
        return m_window;
    }

    glm::vec3 verts[8] = {
        glm::vec3(-0.5,-0.5,-0.5),
        glm::vec3(-0.5,-0.5,0.5),
        glm::vec3(0.5,-0.5,0.5),
        glm::vec3(0.5,-0.5,-0.5),
        glm::vec3(-0.5,0.5,-0.5),
        glm::vec3(-0.5,0.5,0.5),
        glm::vec3(0.5,0.5,0.5),
        glm::vec3(0.5,0.5,-0.5)
    };

    

    unsigned int indices[36] = {
        //bottom
        0,2,1,
        0,3,2,
        //left
        0,1,4,
        4,1,5,
        //front
        1,2,5,
        5,2,6,
        //right
        3,7,6,
        3,6,2,
        //back
        0,4,3,
        4,7,3,
        //top
        4,5,6,
        7,4,6

    };


private:
    GLFWwindow* m_window;
    Shader m_shader;
    Camera m_camera;
    unsigned int VAO;

};