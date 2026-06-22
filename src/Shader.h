#pragma once

#include <glad/glad.h>

// Minimal shader-program helper. Fill in loading/compiling as needed.
class Shader {
public:
    GLuint ID = 0;

    Shader() = default;

    void init(const char* vertexPath, const char* fragmentPath) {
        (void)vertexPath; (void)fragmentPath;
        // TODO: read, compile, link.
    }

    void use() const { glUseProgram(ID); }
};