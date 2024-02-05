#pragma once

#include <Eigen/Core>
#include <Eigen/SparseCore>

#include <cstdint>
#include <string>
#include <vector>

using EdgeId = std::uint64_t;
using VertexId = std::uint64_t;
using FaceId = std::uint64_t;
using Orientation = std::int32_t;

struct Mesh {
    Eigen::ArrayX3d vertexPositions;
    Eigen::ArrayX3d faceNormals;
    Eigen::SparseMatrix<Orientation> edges;
    Eigen::SparseMatrix<Orientation> faces;

    static auto ReadObj(const std::string& filePath) -> Mesh;
};