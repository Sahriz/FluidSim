#pragma once

#include <glad/glad.h>
#include "glm.hpp"
#include "gtc/type_ptr.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <unordered_map>



// Minimal shader-program helper. Fill in loading/compiling as needed.
class Shader {
public:
    GLuint ID = 0;

    Shader() = default;
    ~Shader() { if (ID) glDeleteProgram(ID); }
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& o) noexcept : ID(o.ID), m_locs(std::move(o.m_locs)) { o.ID = 0; }
    Shader& operator=(Shader&& o) noexcept { 
        if (this != &o) {
            if (ID) glDeleteProgram(ID);
            ID = o.ID; o.ID = 0;
            m_locs = std::move(o.m_locs);
        }
        return *this; }

    bool init(const std::string& vertexPath,const std::string& fragmentPath) {
        GLuint vert = compileStage(GL_VERTEX_SHADER, vertexPath);
        if (!vert) return false;
        GLuint frag = compileStage(GL_FRAGMENT_SHADER, fragmentPath);
        if (!frag) { glDeleteShader(vert); return false; }

        GLuint prog = glCreateProgram();
        glAttachShader(prog, vert);
        glAttachShader(prog, frag);
        bool ok = link(prog, vertexPath + " + " + fragmentPath);
        glDeleteShader(vert);
        glDeleteShader(frag);
        if (!ok) { glDeleteProgram(prog); return false; }

        adopt(prog);
        return true;
    }
    bool init(const std::string& compPath) {
        GLuint comp = compileStage(GL_COMPUTE_SHADER, compPath);
        if (!comp) return false; 

        GLuint prog = glCreateProgram();
        glAttachShader(prog, comp);
        bool ok = link(prog, compPath);
        glDeleteShader(comp);
        if (!ok) { glDeleteProgram(prog); return false; }

        adopt(prog);
        return true;
    }
    void use() const { glUseProgram(ID); }

    void set(const std::string& n, float v) const {
        glProgramUniform1f(ID, loc(n), v);
    }
    void set(const std::string& n, int v) const {
        glProgramUniform1i(ID, loc(n), v);
    }
    void set(const std::string& n, const glm::vec3& v) const {
        glProgramUniform3fv(ID, loc(n), 1, &v.x);
    }
    void set(const std::string& n, const glm::mat4& m) const {
        glProgramUniformMatrix4fv(ID, loc(n), 1, GL_FALSE, glm::value_ptr(m));
    }

private:
    mutable std::unordered_map<std::string, GLint> m_locs;

    GLint loc(const std::string& name) const {
        auto it = m_locs.find(name);
        if (it != m_locs.end()) return it->second;
        GLint l = glGetUniformLocation(ID, name.c_str());
        if (l < 0) std::cerr << "[Shader] uniform not found (or unused): " << name << "\n";
        m_locs[name] = l;
        return l;
    }

    static std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file) { std::cerr << "[Shader] failed to open: " << path << "\n"; return{}; }

        std::stringstream ss;
        ss << file.rdbuf();
        return ss.str();

    }
    static std::string preprocess(const std::string& source, const std::string& dir) {
        std::stringstream out;
        std::istringstream in(source);
        std::string line;
        int lineNum = 0;
        while (std::getline(in, line)) {
            lineNum++;
            if (line.rfind("#include", 0) == 0) {
                size_t a = line.find('"'), b = line.rfind('"');
                if (a == std::string::npos || b <= a) {
                    std::cerr << "[Shader] bad #include syntax: " << line << "\n";
                    continue;
                }
                out << "#line 1\n";

                out << readFile(dir + line.substr(a + 1, b - a - 1)) << "\n";
                out << "#line " << (lineNum + 1) << "\n";
            }
            else {
                out << line << "\n";
            }
        }
        return out.str();
        
    }

    GLuint compileStage(GLenum type, const std::string& path) {
        std::string raw = readFile(path);
        if (raw.empty()) return 0;

        std::string dir = path.substr(0, path.find_last_of("/\\") + 1);
        std::string source = preprocess(raw, dir);
        if (source.empty()) return 0;

        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint ok = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            std::string log(len > 0 ? len : 1, '\0');
            glGetShaderInfoLog(shader, len, nullptr, log.data());
            std::cerr << "[Shader] compile failed: " << path << "\n" << log << "\n";
            return 0;
        }
        return shader;
    }
    static bool link(GLuint prog, const std::string& label) {
        glLinkProgram(prog);
        GLint ok = GL_FALSE;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
            std::string log(len > 0 ? len : 1, '\0');
            glGetProgramInfoLog(prog, len, nullptr, log.data());
            std::cerr << "[Shader] link failed: " << label << "\n" << log << "\n";
            return false;
        }
        return true;
    }

    void adopt(GLuint newProg) {
        if (ID) glDeleteProgram(ID);
        ID = newProg;
        m_locs.clear();
    }
};