#include "meshcomponent.h"
#include "GLMTestHelpers.h"

#include <gtest/gtest.h>

#include <string_view>

using namespace std::string_view_literals;

TEST(MeshTest, TriangleParse) {

    auto mesh = Mesh::ReadObj(
R"(# Blender 4.0.2
# www.blender.org
o Plane
v -1.000000 0.000000 1.000000
v 1.000000 0.000000 1.000000
v -1.000000 0.000000 -1.000000
vn -0.0710 0.9949 0.0710
vn -0.0000 1.0000 -0.0000
vn -0.0000 0.9999 -0.0121
s 0
f 1//1 2//3 3//2
)"sv);
    
    EXPECT_EQ(mesh.vertices.size(), 3);
    
    GLM_EXPECT_NEAR(mesh.vertices[0].position, glm::vec3(-1.0, 0.0, 1.0), 1e-5);
    GLM_EXPECT_NEAR(mesh.vertices[1].position, glm::vec3( 1.0, 0.0, 1.0), 1e-5);
    GLM_EXPECT_NEAR(mesh.vertices[2].position, glm::vec3(-1.0, 0.0,-1.0), 1e-5);

    GLM_EXPECT_NEAR(mesh.vertices[0].normal, glm::vec3(-0.0710, 0.9949, 0.0710), 1e-5);
    GLM_EXPECT_NEAR(mesh.vertices[1].normal, glm::vec3( 0.0000, 0.9999,-0.0121), 1e-5);
    GLM_EXPECT_NEAR(mesh.vertices[2].normal, glm::vec3( 0.0000, 1.0000, 0.0000), 1e-5);
}
TEST(MeshTest, QuadParse) {

    auto mesh = Mesh::ReadObj(
R"(# Blender 4.0.2
# www.blender.org
o Plane
v -1.000000 0.000000 1.000000
v 1.000000 0.000000 1.000000
v -1.000000 0.000000 -1.000000
v 1.000000 0.000000 -1.000000
vn -0.0710 0.9949 0.0710
vn -0.0000 1.0000 -0.0000
vn -0.0000 0.9999 -0.0121
s 0
f 1//1 2//3 3//2
f 3//1 2//1 4//2
)"sv);
    
    EXPECT_EQ(mesh.vertices.size(), 6);
    
    GLM_EXPECT_NEAR(mesh.vertices[0].position, glm::vec3(-1.0, 0.0, 1.0), 1e-5);
    GLM_EXPECT_NEAR(mesh.vertices[1].position, glm::vec3( 1.0, 0.0, 1.0), 1e-5);
    GLM_EXPECT_NEAR(mesh.vertices[2].position, glm::vec3(-1.0, 0.0,-1.0), 1e-5);
    GLM_EXPECT_NEAR(mesh.vertices[3].position, glm::vec3(-1.0, 0.0,-1.0), 1e-5);
    GLM_EXPECT_NEAR(mesh.vertices[4].position, glm::vec3( 1.0, 0.0, 1.0), 1e-5);
    GLM_EXPECT_NEAR(mesh.vertices[5].position, glm::vec3( 1.0, 0.0,-1.0), 1e-5);

    GLM_EXPECT_NEAR(mesh.vertices[0].normal, glm::vec3(-0.0710, 0.9949, 0.0710), 1e-5);
    GLM_EXPECT_NEAR(mesh.vertices[1].normal, glm::vec3( 0.0000, 0.9999,-0.0121), 1e-5);
    GLM_EXPECT_NEAR(mesh.vertices[2].normal, glm::vec3( 0.0000, 1.0000, 0.0000), 1e-5);
    GLM_EXPECT_NEAR(mesh.vertices[3].normal, glm::vec3(-0.0710, 0.9949, 0.0710), 1e-5);
    GLM_EXPECT_NEAR(mesh.vertices[4].normal, glm::vec3(-0.0710, 0.9949, 0.0710), 1e-5);
    GLM_EXPECT_NEAR(mesh.vertices[5].normal, glm::vec3( 0.0000, 1.0000, 0.0000), 1e-5);
}
