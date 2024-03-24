#include "cameracomponent.h"

#include <glm/glm.hpp>

#include <gtest/gtest.h>

#include <print>
#include <ranges>

TEST(CameraComponent, CorrectProjectionAspect) {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    auto comp = CameraComponent{ .fov = 90.0F, .aspect = 1.0F, .nearZ = 0.1F, .farZ = 100.0F };
    EXPECT_NEAR(comp.GetProjection()[0][0], 1.0F, 1e-5F);
    EXPECT_NEAR(comp.GetProjection()[1][1], 1.0F, 1e-5F);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    comp = CameraComponent{ .fov = 90.0F, .aspect = 3.654732F, .nearZ = 0.1F, .farZ = 100.0F };
    EXPECT_NEAR(comp.GetProjection()[0][0], 1.0F / 3.654732F, 1e-5F);
    EXPECT_NEAR(comp.GetProjection()[1][1], 1.0F, 1e-5F);
}

