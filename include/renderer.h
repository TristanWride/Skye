#pragma once

#include "ecsmanager.h"
#include "meshcomponent.h"
#include "shader.h"
#include "window.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

template <typename ECS>
requires SupportsComponent<ECS, MeshComponent>
struct Renderer {
    ECS& ecs;
    Window& window;
    ShaderProgram shaderProgram;

    Renderer(ECS& ecs, Window& window) 
        : ecs{ecs}, window{window}, shaderProgram(Shader(SHADER_DIR "vert.glsl", GL_VERTEX_SHADER), Shader(SHADER_DIR "frag.glsl", GL_FRAGMENT_SHADER)) 
    {}

    auto RenderMeshes() -> void {
        static float x = 0.0f;
        for (auto&& [id, meshComponent] : ecs.GetAll<MeshComponent>()) {
            auto view = glm::mat4(1.0f);
            view = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f));
            view = glm::rotate(view, x, glm::vec3(1.0, 1.0, 0.0));
            x += 0.01f;
            auto proj = glm::perspective(glm::radians(45.0f), window.GetAspectRatio(), 0.1f, 100.0f);

            glUseProgram(shaderProgram.programHandle);
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(proj));
            glBindVertexArray(meshComponent.vao);
            glDrawArrays(GL_TRIANGLES, 0, meshComponent.numVertices);
        }
    }
};