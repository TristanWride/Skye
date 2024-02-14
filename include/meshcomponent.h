#pragma once

#include "debugutils.h"
#include "opaquetypes.h"

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
    Eigen::ArrayX3<VertexId> faces;

    static auto ReadObj(const char* filePath) -> Mesh;
};

struct MeshComponent {
    Mesh mesh{};
};

struct MeshComponentManager {
    using ComponentId = GenericId;
    using ComponentType = MeshComponent;

    decltype(auto) AllEntitiesAndComponents(this auto&& self) {
        return std::vector<std::pair<EntityId, MeshComponent&>>{};
    }
};