#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

struct Mesh {
    using VertexId = std::uint32_t;

    std::vector<Vertex> vertices;

    static auto ReadObj(const char* filePath) -> Mesh;
};

struct MeshComponent {
    GLuint vbo = 0u;
    GLuint vao = 0u;
    unsigned int numVertices;

    MeshComponent(const Mesh& mesh) noexcept;
    MeshComponent(MeshComponent&& other) noexcept;

    ~MeshComponent() noexcept;
};
