#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <iostream>

class Window {
public:
	Window() = default;

    static void APIENTRY myCallback(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei length,
        const GLchar* message, const void* userParam) {

        (void)length; (void)userParam;

        if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
            return;
        }

        const char* src;
        switch (source) {
        case GL_DEBUG_SOURCE_API: src = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: src = "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: src = "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: src = "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION: src = "Application"; break;
        default: src = "Other"; break;
        }

        const char* typ;
        switch (type) {
        case GL_DEBUG_TYPE_ERROR: typ = "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typ = "Deprecated"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: typ = "Undefined Behavior"; break;
        case GL_DEBUG_TYPE_PORTABILITY: typ = "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE: typ = "Performance"; break;

        }

        const char* sev;
        switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: sev = "HIGH"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: sev = "MEDIUM";  break;
        case GL_DEBUG_SEVERITY_LOW: sev = "LOW"; break;
        default: sev = "INFO"; break;
        }

        std::cout << "[GL][" << sev << "] " << typ << " from " << src << " (id=" << id << "): " << message << std::endl;

    }

	bool init(int width, int height, const char* title) {
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		window = glfwCreateWindow(width, height, title, NULL, NULL);

        if (window == NULL) {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);   // fire on the calling thread → usable stack traces
        glDebugMessageCallback(myCallback, nullptr);

        glViewport(0, 0, width, height);

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        return true;
	}


	GLFWwindow* window = nullptr;
};