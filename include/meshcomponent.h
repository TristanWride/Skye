#pragma once

#include "debugutils.h"

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <algorithm>
#include <string>
#include <sstream>
#include <format>
#include <vector>
#include <ranges>
#include <utility>
#include <cctype>
#include <string_view>
#include <charconv>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

struct Mesh {
    using VertexId = std::uint32_t;

    std::vector<Vertex> vertices;

    [[nodiscard]] static auto ReadObj(const char* filePath) -> Mesh;
    [[nodiscard]] static auto ReadObj(std::string_view inputStr) -> Mesh;
    [[nodiscard]] static auto ReadObj(auto&& inputFile) -> Mesh {
        auto mesh = Mesh{};

        auto indexedVertices = std::vector<glm::vec3>{};
        auto indexedNormals = std::vector<glm::vec3>{};

        // Read data from file into std::vectors
        for (std::string line; std::getline(inputFile, line); ){
            auto iss = std::stringstream{line};
            std::string rowType;
            iss >> rowType;
            if (rowType == "#") {
                continue;
            } else if (rowType == "o") {
                continue;
            } else if (rowType == "s") {
                continue;
            } else if (rowType == "v") {
                auto& vertex = indexedVertices.emplace_back();
                iss >> vertex.x >> vertex.y >> vertex.z;
            } else if (rowType == "vn") {
                auto& normal = indexedNormals.emplace_back();
                iss >> normal.x >> normal.y >> normal.z;
            } else if (rowType == "f") {
                auto faceData = std::ranges::istream_view<std::string>(iss)
                | std::views::transform([](const auto& entry) -> std::tuple<VertexId, std::uint64_t> {
                    auto vertexEnd = std::ranges::find(entry, '/');
                    auto normalBegin = std::find_if(vertexEnd, entry.cend(), [](const auto& c) { return c != '/'; });
                    return {
                        std::stoul(std::string(entry.begin(), vertexEnd)) - 1,
                        std::stoul(std::string(normalBegin, entry.end())) - 1
                    };
                }) | std::ranges::to<std::vector>();

                if (faceData.size() > 3) {
                    DebugMessage("ERROR", "Non-triangle faces not supported");
                    continue;
                }

                std::ranges::copy(
                    faceData | std::views::transform([&indexedVertices, &indexedNormals](auto& entry) {
                        return Vertex{ 
                            indexedVertices[std::get<0>(entry)],
                            indexedNormals[std::get<1>(entry)]
                        };
                    }),
                    std::back_inserter(mesh.vertices)
                );
            } else {
                DebugMessage("WARN", std::format("Encountered unknown row type {}", rowType));
            }
        }

        return mesh;
    }
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
