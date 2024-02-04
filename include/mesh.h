#include <Eigen/Core>

#include <cstdint>

using EdgeId = std::uint64_t;
using VertexId = std::uint64_t;
using FaceId = std::uint64_t;

struct Edge {
    EdgeId id;
    EdgeId next;
    EdgeId prev;
    EdgeId twin;
    FaceId face;
    VertexId vertex;
};

struct Vertex {
    VertexId id;
    EdgeId edge;
};

struct Face {
    FaceId id;
    EdgeId edge;
};

struct Mesh {
    Eigen::ArrayX3d vertices;
    Eigen::ArrayX3d normals;
};