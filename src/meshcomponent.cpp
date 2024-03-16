#include "meshcomponent.h"
#include "debugutils.h"

#include <array>
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <string>
#include <sstream>
#include <optional>
#include <ranges>
#include <vector>


auto Mesh::ReadObj(const char* filePath) -> Mesh {
    DebugMessage("INFO", std::format("Reading object file \"{}\"", filePath));
    auto inputFile = std::ifstream(filePath);

    if (!inputFile.is_open()) {
        DebugMessage("ERROR", std::format("Failed to open file \"{}\"", filePath));
        return Mesh{};
    }

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

    DebugMessage("INFO", std::format("Found {} vertices", mesh.vertices.size()));

    return mesh;
}

MeshComponent::MeshComponent(const Mesh& mesh) noexcept
    : numVertices{static_cast<unsigned int>(mesh.vertices.size())}
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);
}

MeshComponent& MeshComponent::operator=(MeshComponent&& other) noexcept {
    vao = std::exchange(other.vao, 0);
    vbo = std::exchange(other.vbo, 0);
    numVertices = other.numVertices;
    return *this;
}

MeshComponent::MeshComponent(MeshComponent&& other) noexcept
    : vao{other.vao}, vbo{other.vbo}, numVertices{other.numVertices}
{
    other.vao = 0;
    other.vbo = 0;
}

MeshComponent::~MeshComponent() noexcept {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}