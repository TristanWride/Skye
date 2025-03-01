#pragma once

#include <glm/glm.hpp>

#include <gtest/gtest.h>

#define GLM_EXPECT_NEAR(a, b, absError) \
    EXPECT_NEAR(a.x, b.x, absError);\
    EXPECT_NEAR(a.y, b.y, absError);\
    EXPECT_NEAR(a.z, b.z, absError)
