#pragma once

#include <Eigen/Core>

#include <cstdint>
#include <vector>

struct Mesh {
    using ScalarType = float;
    using EdgeId = std::uint32_t;
    using VertexId = std::uint32_t;
    using FaceId = std::uint32_t;

    Eigen::ArrayX3<ScalarType> vertices;
    Eigen::ArrayX3<ScalarType> faceNormals;
    std::vector<std::vector<VertexId>> faces;

    static auto ReadObj(const char* filePath) -> Mesh;
};

struct MeshComponent {
    Mesh mesh{};
};