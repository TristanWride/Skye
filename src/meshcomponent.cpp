#include "meshcomponent.h"

#include "debugutils.h"

#include <bit>
#include <cstddef>
#include <format>
#include <fstream>
#include <limits>
#include <sstream>
#include <string_view>
#include <utility>
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
    auto objStream = std::stringstream{};
    objStream << inputStr;
    return Mesh::ReadObj(objStream);
}

MeshComponent::MeshComponent(const Mesh& mesh) noexcept
    : numVertices{static_cast<unsigned int>(mesh.vertices.size())}
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    const auto meshSize = [&] {
        if (mesh.vertices.size() * sizeof(Vertex) > static_cast<size_t>(std::numeric_limits<GLsizeiptr>::max())) [[unlikely]] {
            DebugMessage("WARN", "Mesh loaded is too large, cropping to size");
            return std::numeric_limits<GLsizeiptr>::max();
        }

        return static_cast<GLsizeiptr>(mesh.vertices.size() * sizeof(Vertex));
    }();

    glBufferData(GL_ARRAY_BUFFER, meshSize, mesh.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), std::bit_cast<void *>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), std::bit_cast<void *>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);
}

auto MeshComponent::operator=(MeshComponent&& other) noexcept -> MeshComponent& {
    if (vao != 0U) { glDeleteVertexArrays(1U, &vao); }
    if (vbo != 0U) { glDeleteVertexArrays(1U, &vbo); }

    vao = std::exchange(other.vao, 0U);
    vbo = std::exchange(other.vbo, 0U);
    numVertices = other.numVertices;
    return *this;
}

MeshComponent::MeshComponent(MeshComponent&& other) noexcept
    : vao{other.vao}, vbo{other.vbo}, numVertices{other.numVertices}
{
    other.vao = 0U;
    other.vbo = 0U;
}

MeshComponent::~MeshComponent() noexcept {
    if (vao != 0U) { glDeleteVertexArrays(1U, &vao); }
    if (vbo != 0U) { glDeleteVertexArrays(1U, &vbo); }
}