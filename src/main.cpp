#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <format>

#include "debugutils.h"
#include "mesh.h"
#include "window.h"

auto ErrorCallback(int error, const char* description) -> void {
    DebugMessage("ERROR", std::format("error code {}: {}", error, description));
}

auto GlobalSetup() -> bool {
    if (glfwInit() != GLFW_TRUE) {
        DebugMessage("ERROR", "Failed to initialize GLFW");
        goto failglfwinit;
    }

    glfwSetErrorCallback(ErrorCallback);

    return true;

failglfwinit:
    return false;
}

auto GlobalCleanup() -> bool {
    glfwTerminate();

    return true;
}

auto main() -> int {
    if (!GlobalSetup()) {
        DebugMessage("ERROR", "Failed setup");
        return EXIT_FAILURE;
    }

    {
        auto mesh = Mesh::ReadObj(SOURCEDIR "/data/tris.obj");
        auto window = Window();
        gladLoadGL();

        while (!glfwWindowShouldClose(window.window)) {
            glClearColor(0.7f, 0.9f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glfwSwapBuffers(window.window);

            glfwPollEvents();
        }
    }

    return EXIT_SUCCESS;
}