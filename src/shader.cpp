#include "shader.h"

#include <fstream>

auto CheckShaderCompilation(GLuint shaderHandle) {
    auto success = GLint{};
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);
    if (!success) {
        auto infoLogLength = GLint{};
        glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infoLogLength);
        auto infoLog = std::string(infoLogLength, '\0');
        glGetShaderInfoLog(shaderHandle, infoLog.length(), NULL, &infoLog[0]);
        ThrowMessage("ERROR", std::format("Shader compilation error:\n{}", infoLog));
    }
}

Shader::Shader(const char* fileName, GLenum shaderType) {
    auto shaderFile = std::ifstream(fileName, std::ifstream::ate);
    if (!shaderFile.is_open()) ThrowMessage("ERROR", std::format("Couldn't open file \"{}\"", fileName));
    auto length = shaderFile.tellg();
    shaderFile.seekg(0);
    shaderSource.assign(length, '\0');
    shaderFile.read(&shaderSource[0], length);

    shaderHandle = glCreateShader(shaderType);
    const auto* shaderSourceCStr = shaderSource.c_str();

    glShaderSource(shaderHandle, 1, &shaderSourceCStr, NULL);
    glCompileShader(shaderHandle);
    CheckShaderCompilation(shaderHandle);
}

Shader::~Shader() {
    glDeleteShader(shaderHandle);
}

ShaderProgram::~ShaderProgram() {
    glDeleteProgram(programHandle);
}