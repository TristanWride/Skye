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

    [[nodiscard]] static auto ReadObj(const char* filePath) -> Mesh;
};

struct MeshComponent {
    GLuint vbo = 0u;
    GLuint vao = 0u;
    unsigned int numVertices;

    [[nodiscard]] MeshComponent(const Mesh& mesh) noexcept;

    [[nodiscard]] MeshComponent& operator=(MeshComponent&& other) noexcept;
    [[nodiscard]] MeshComponent(MeshComponent&& other) noexcept;

    ~MeshComponent() noexcept;
};
