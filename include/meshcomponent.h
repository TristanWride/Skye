#pragma once

#include <glad/glad.h>
#include <Eigen/Core>

struct Mesh {
    using ScalarType = float;
    using EdgeId = std::uint32_t;
    using VertexId = std::uint32_t;
    using FaceId = std::uint32_t;

    Eigen::ArrayX3<ScalarType> vertexPositions;
    Eigen::ArrayX3<ScalarType> faceNormals;
    Eigen::ArrayX3<VertexId> faces;

    static auto ReadObj(const char* filePath) -> Mesh;
};

struct VertexData {
    std::array<Mesh::ScalarType, 3> position;
    std::array<Mesh::ScalarType, 3> normal;
};

struct MeshComponent {
    GLuint vbo;
    GLuint vao;
    unsigned int numVertices;

    MeshComponent(const Mesh& mesh);
    MeshComponent(MeshComponent&& other);

    ~MeshComponent();
};
