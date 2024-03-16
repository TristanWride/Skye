#pragma once

#include "ecsmanager.h"
#include "meshcomponent.h"
#include "shader.h"
#include "window.h"
#include "cameracomponent.h"
#include "transformcomponent.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

template <typename ECS>
struct Renderer {
    ECS& ecs;
    Window& window;
    ShaderProgram shaderProgram;
    std::optional<EntityId> activeCamera;

    Renderer(ECS& ecs, Window& window) 
        : ecs{ecs}, window{window}, shaderProgram(Shader(SHADER_DIR "vert.glsl", GL_VERTEX_SHADER), Shader(SHADER_DIR "frag.glsl", GL_FRAGMENT_SHADER)) 
    {}

    auto RenderMeshes() noexcept -> void {
        static float x = 0.0f;

        if (!activeCamera) {
            DebugMessage("ERROR", "No active camera found");
            return;
        }

        const auto& camera = ecs.GetComponent<CameraComponent>(activeCamera.value());
        const auto& cameraTransform = ecs.GetComponent<TransformComponent>(activeCamera.value());

        auto view = cameraTransform.GetInverseTransform();
        auto proj = camera.GetProjection();

        for (auto&& [id, meshComponent] : ecs.GetAll<MeshComponent>()) {
            glUseProgram(shaderProgram.programHandle);
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(proj));
            glBindVertexArray(meshComponent.vao);
            glDrawArrays(GL_TRIANGLES, 0, meshComponent.numVertices);
        }
    }
};