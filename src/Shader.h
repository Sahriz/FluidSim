#pragma once

#include <glad/glad.h>
#include <fstream>
#include <filesystem>
#include <iostream>


// Minimal shader-program helper. Fill in loading/compiling as needed.
class Shader {
public:
    GLuint ID = 0;

    Shader() = default;

    void init(std::string vertexPath, std::string fragmentPath) {
        

        std::string vertShaderContent = getShaderContentFromFile(vertexPath);
        std::string fragShaderContent = getShaderContentFromFile(fragmentPath);

        unsigned int vertexShader;
        vertexShader = glCreateShader(GL_VERTEX_SHADER);

        const char* vertSrc = vertShaderContent.c_str();

        glShaderSource(vertexShader, 1, &vertSrc, NULL);
        glCompileShader(vertexShader);

        unsigned int fragShader;
        fragShader = glCreateShader(GL_FRAGMENT_SHADER);

        const char* fragSrc = fragShaderContent.c_str();

        glShaderSource(fragShader, 1, &fragSrc, NULL);
        glCompileShader(fragShader);

        ID = glCreateProgram();

        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragShader);
        glLinkProgram(ID);

        glDeleteShader(vertexShader);
        glDeleteShader(fragShader);

    }

    std::string getShaderContentFromFile(std::string path) {
        size_t fileSize = std::filesystem::file_size(path);
        std::string content(fileSize, '\0');
        std::ifstream in(path);
        in.read(&content[0], fileSize);
        return content;
    }



    void use() const { glUseProgram(ID); }
};