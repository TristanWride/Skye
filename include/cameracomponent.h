#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct CameraComponent {
    float fov;
    float aspect;
    float nearZ;
    float farZ;

    [[nodiscard]] inline auto GetProjection() const noexcept -> glm::mat4 {
        return glm::perspective(glm::radians(fov), aspect, nearZ, farZ);
    }
};