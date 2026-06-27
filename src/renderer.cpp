#include "renderer.h"

Renderer::Renderer() {}
Renderer::~Renderer() {}

void Renderer::tempCode() {
    

    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


   

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    

}

void APIENTRY myCallback(GLenum source, GLenum type, GLuint id,
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


bool Renderer::init(int width, int height, const char* title) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    m_window = glfwCreateWindow(width, height, title, NULL, NULL);

    if (m_window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);   // fire on the calling thread → usable stack traces
    glDebugMessageCallback(myCallback, nullptr);

    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

    
    std::string vertPath = std::string(SHADER_DIR) + "render.vert";
    std::string fragPath = std::string(SHADER_DIR) + "render.frag";
    
    m_shader.init(vertPath, fragPath);

    tempCode();
    glUseProgram(m_shader.ID);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    float aspectRatio = float(width) / float(height);
    m_camera.Init(70, aspectRatio, 0.1, 100, m_shader.ID);

    return true;

}

void Renderer::render(FluidSim& system, frameInput& fInput) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_camera.Update(fInput);

    glUseProgram(m_shader.ID);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, std::size(indices), GL_UNSIGNED_INT, (void*)0);
}

void Renderer::cleanup() {
    // TODO: release GL resources.
}

void Renderer::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}