#include "shader.h"

#include "debugutils.h"

#include <glad/glad.h>

#include <format>
#include <fstream>
#include <string>
#include <utility>

Shader::Shader(const char* fileName, GLenum shaderType) {
    auto shaderFile = std::ifstream(fileName, std::ifstream::ate);

    if (!shaderFile.is_open()) {
        DebugMessage("ERROR", "Couldn't open file \"{}\"", fileName);
        return;
    }

    auto length = shaderFile.tellg();
    shaderFile.seekg(0);
    auto shaderSource = std::string(length, '\0');
    shaderFile.read(shaderSource.data(), length);

    shaderHandle = glCreateShader(shaderType);
    const auto* shaderSourceCStr = shaderSource.c_str();

    glShaderSource(shaderHandle, 1, &shaderSourceCStr, nullptr);
    glCompileShader(shaderHandle);

    auto success = GLint{};
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);
    if (!static_cast<bool>(success)) {
        auto infoLogLength = GLint{};
        glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infoLogLength);
        auto infoLog = std::string(infoLogLength, '\0');
        glGetShaderInfoLog(shaderHandle, infoLogLength, nullptr, infoLog.data());
        DebugMessage("ERROR", "Shader compilation error:\n{}", infoLog);
        shaderHandle = 0U;
        return;
    }
}

auto Shader::operator=(Shader&& other) noexcept -> Shader& {
    if (shaderHandle != 0U) { glDeleteShader(shaderHandle); }
    shaderHandle = std::exchange(other.shaderHandle, 0U);
    return *this;
}

Shader::Shader(Shader&& other) noexcept
    : shaderHandle{other.shaderHandle}
{
    other.shaderHandle = 0U;
}

Shader::~Shader() noexcept {
    if(shaderHandle != 0U) { glDeleteShader(shaderHandle); }
}

ShaderProgram::~ShaderProgram() noexcept {
    glDeleteProgram(programHandle);
}