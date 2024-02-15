#pragma once

#include "debugutils.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <string>

struct Shader {
    GLuint shaderHandle;

    Shader(const char* fileName, GLenum shaderType);
    ~Shader();

    Shader() = delete;
    Shader(const Shader& other) = delete;
    Shader& operator=(const Shader& other) = delete;
    Shader(Shader&& other) = delete;
    Shader& operator=(Shader&& other) = delete;

private:
    std::string shaderSource;
};

struct ShaderProgram {
    GLuint programHandle;

    template <typename... T>
    ShaderProgram(T&&... shaders) {
        programHandle = glCreateProgram();

        ((glAttachShader(programHandle, shaders.shaderHandle)), ...);

        glLinkProgram(programHandle);

        auto CheckShaderProgramLinking = [](GLuint programHandle) {
            auto success = GLint{};
            glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
            if (!success) {
                auto infoLogLength = GLint{};
                glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &infoLogLength);
                auto infoLog = std::string(infoLogLength, '\0');
                glGetProgramInfoLog(programHandle, infoLog.length(), NULL, &infoLog[0]);
                ThrowMessage("ERROR", std::format("Shader linking error:\n{}", infoLog));
            }
        }; 

        CheckShaderProgramLinking(programHandle);
    }

    ~ShaderProgram();

    ShaderProgram(const ShaderProgram& other) = delete;
    ShaderProgram& operator=(const ShaderProgram& other) = delete;
    ShaderProgram(ShaderProgram&& other) = delete;
    ShaderProgram& operator=(ShaderProgram&& other) = delete;
};