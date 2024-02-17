#pragma once

#include <Eigen/Core>

#include <cstdint>
#include <vector>
#include <optional>

struct Mesh {
    using ScalarType = float;
    using EdgeId = std::uint32_t;
    using VertexId = std::uint32_t;
    using FaceId = std::uint32_t;

    struct Renderable {
        Eigen::Array3X<ScalarType> verticesBuff;
        Eigen::Array3X<ScalarType> normalsBuff;

        Renderable() = default;
        Renderable(VertexId numVertices);
    };

    Eigen::ArrayX3<ScalarType> vertices;
    Eigen::ArrayX3<ScalarType> faceNormals;
    Eigen::ArrayX3<VertexId> faces;

    decltype(auto) GetRenderable(this auto&& self) {
        if (!self.dirty) return std::forward<decltype(self)>(self).renderable;
        std::forward<decltype(self)>(self).UpdateRenderable();
        return std::forward<decltype(self)>(self).renderable;
    }

    auto UpdateRenderable() -> void;

    static auto ReadObj(const char* filePath) -> Mesh;

private:
    bool dirty{true};
    Renderable renderable;
};

struct MeshComponent {
    Mesh mesh{};
};