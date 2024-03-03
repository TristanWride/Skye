#pragma once

#include "debugutils.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

struct Shader {
    GLuint shaderHandle = 0u;

    Shader(const char* fileName, GLenum shaderType);
    ~Shader() noexcept;

    Shader() = delete;
    Shader(const Shader& other) = delete;
    Shader& operator=(const Shader& other) = delete;

    Shader(Shader&& other) noexcept;
};

struct ShaderProgram {
    GLuint programHandle = 0u;

    template <typename... T>
    requires (std::convertible_to<T, Shader> && ...)
    ShaderProgram(T&&... shaders) {
        if (((shaders.shaderHandle == 0u) || ...)) {
            DebugMessage("ERROR", "Incomplete shader given to program");
            return;
        }

        programHandle = glCreateProgram();

        (glAttachShader(programHandle, shaders.shaderHandle), ...);

        glLinkProgram(programHandle);

        auto CheckShaderProgramLinking = [](GLuint programHandle) {
            auto success = GLint{};
            glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
            if (!success) {
                auto infoLogLength = GLint{};
                glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &infoLogLength);
                auto infoLog = std::string(infoLogLength, '\0');
                glGetProgramInfoLog(programHandle, infoLogLength, NULL, &infoLog[0]);
                ThrowMessage("ERROR", std::format("Shader linking error:\n{}", infoLog));
            }
        }; 

        CheckShaderProgramLinking(programHandle);
    }

    ~ShaderProgram() noexcept;

    ShaderProgram(const ShaderProgram& other) = delete;
    ShaderProgram& operator=(const ShaderProgram& other) = delete;
    ShaderProgram(ShaderProgram&& other) = delete;
    ShaderProgram& operator=(ShaderProgram&& other) = delete;
};