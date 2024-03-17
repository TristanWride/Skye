#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct TransformComponent {
    glm::vec3 scale;
    glm::mat4 rotation;
    glm::vec3 translation;

    [[nodiscard]] inline auto GetTransform() const noexcept -> glm::mat4 {
        auto t = glm::mat4(1.0f);
        t = glm::scale(t, scale);
        t = rotation * t;
        t = glm::translate(t, translation);
        return t;
    }

    [[nodiscard]] inline auto GetInverseTransform() const noexcept -> glm::mat4 {
        auto t = glm::mat4(1.0f);
        t = glm::translate(t, -translation);
        t = glm::transpose(rotation) * t;
        t = glm::scale(t, 1.0f / scale);
        return t;
    }
};