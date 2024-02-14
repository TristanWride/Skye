#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "debugutils.h"
#include "ecsmanager.h"
#include "meshcomponent.h"
#include "window.h"
#include "shader.h"

#include <format>

auto ErrorCallback(int error, const char* description) -> void {
    DebugMessage("ERROR", std::format("error code {}: {}", error, description));
}

auto GlobalSetup() -> void {
    if (glfwInit() != GLFW_TRUE) ThrowMessage("ERROR", "Failed to initialize GLFW");
    glfwSetErrorCallback(ErrorCallback);
}

auto GlobalCleanup() -> void {
    glfwTerminate();
}

auto main() -> int {
    struct Environment {
        Environment() {
            GlobalSetup();
        }

        ~Environment() {
            GlobalCleanup();
        }
    }; 
    auto env = Environment{};
    auto window = Window();

    if (!gladLoadGL()) {
        ThrowMessage("ERROR", "Failed to initialise GLAD");
    }

    float vertices[] = {
        -.5f, -.5f, .0f,  1.f, .0f, .0f,
         .5f, -.5f, .0f,  .0f, 1.f, .0f,
         .0f,  .5f, .0f, .0f, .0f, 1.f
    };
    unsigned int indices[] = {
        0, 1, 2
    };

    auto program = ShaderProgram{
        Shader(SHADER_DIR "vert.glsl", GL_VERTEX_SHADER),
        Shader(SHADER_DIR "frag.glsl", GL_FRAGMENT_SHADER)
    };

    auto vbo = GLuint{};
    auto ebo = GLuint{};
    auto vao = GLuint{};
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenVertexArrays(1, &vao);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    auto coreManager = ECSManager<MeshComponent>{};

    auto entity = coreManager.FreshEntity();

    while (!glfwWindowShouldClose(window.window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program.programHandle);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window.window);

        glfwPollEvents();
    }

    return EXIT_SUCCESS;
}