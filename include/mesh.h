#pragma once

#include <Eigen/Core>
#include <Eigen/SparseCore>

#include <cstdint>
#include <string>
#include <vector>

using EdgeId = std::uint64_t;
using VertexId = std::uint64_t;
using FaceId = std::uint64_t;

struct Mesh {
    using ScalarType = double;
    Eigen::ArrayX3<ScalarType> vertices;
    Eigen::ArrayX3<ScalarType> faceNormals;
    std::vector<std::vector<VertexId>> faces;

    static auto ReadObj(const char* filePath) -> Mesh;
};