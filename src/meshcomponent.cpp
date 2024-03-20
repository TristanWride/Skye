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

    return Mesh::ReadObj(inputFile);
}

auto Mesh::ReadObj(std::string_view inputStr) -> Mesh {
    auto ss = std::stringstream{};
    ss << inputStr;
    return Mesh::ReadObj(ss);
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
    if (vao != 0u) glDeleteVertexArrays(1, &vao);
    if (vbo != 0u) glDeleteVertexArrays(1, &vbo);

    vao = std::exchange(other.vao, 0u);
    vbo = std::exchange(other.vbo, 0u);
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
    if (vao != 0u) glDeleteVertexArrays(1, &vao);
    if (vbo != 0u) glDeleteVertexArrays(1, &vbo);
}