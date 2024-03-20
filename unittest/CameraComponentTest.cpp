#include "cameracomponent.h"

#include <glm/glm.hpp>

#include <gtest/gtest.h>

#include <print>
#include <ranges>

TEST(CameraComponent, CorrectProjectionAspect) {
    auto comp = CameraComponent{
        .fov = 90.0f,
        .aspect = 1.0f,
        .nearZ = 0.1f,
        .farZ = 100.0f
    };


    EXPECT_NEAR(comp.GetProjection()[0][0], 1.0f, 1e-5f);
    EXPECT_NEAR(comp.GetProjection()[1][1], 1.0f, 1e-5f);

    comp = CameraComponent{
        .fov = 90.0f,
        .aspect = 3.654732f,
        .nearZ = 0.1f,
        .farZ = 100.0f
    };


    EXPECT_NEAR(comp.GetProjection()[0][0], 1.0f / 3.654732f, 1e-5f);
    EXPECT_NEAR(comp.GetProjection()[1][1], 1.0f, 1e-5f);
}

