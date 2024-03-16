#include "shader.h"

#include <fstream>
#include <string>

Shader::Shader(const char* fileName, GLenum shaderType) {
    auto shaderFile = std::ifstream(fileName, std::ifstream::ate);

    if (!shaderFile.is_open()) {
        DebugMessage("ERROR", std::format("Couldn't open file \"{}\"", fileName));
        return;
    }

    auto length = shaderFile.tellg();
    shaderFile.seekg(0);
    auto shaderSource = std::string(length, '\0');
    shaderFile.read(&shaderSource[0], length);

    shaderHandle = glCreateShader(shaderType);
    const auto* shaderSourceCStr = shaderSource.c_str();

    glShaderSource(shaderHandle, 1, &shaderSourceCStr, NULL);
    glCompileShader(shaderHandle);

    auto success = GLint{};
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);
    if (!success) {
        auto infoLogLength = GLint{};
        glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infoLogLength);
        auto infoLog = std::string(infoLogLength, '\0');
        glGetShaderInfoLog(shaderHandle, infoLogLength, NULL, &infoLog[0]);
        DebugMessage("ERROR", std::format("Shader compilation error:\n{}", infoLog));
        shaderHandle = 0u;
        return;
    }
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (shaderHandle != 0u) glDeleteShader(shaderHandle);
    shaderHandle = std::exchange(other.shaderHandle, 0u);
    return *this;
}

Shader::Shader(Shader&& other) noexcept
    : shaderHandle{other.shaderHandle}
{
    other.shaderHandle = 0u;
}

Shader::~Shader() noexcept {
    if(shaderHandle != 0u) glDeleteShader(shaderHandle);
}

ShaderProgram::~ShaderProgram() noexcept {
    glDeleteProgram(programHandle);
}