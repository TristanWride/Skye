#include "meshcomponent.h"
#include "debugutils.h"

#include <array>
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <optional>
#include <ranges>
#include <vector>

auto Mesh::ReadObj(const char* filePath) -> Mesh {
    DebugMessage("INFO", std::format("Reading object file \"{}\"", filePath));
    auto inputFile = std::ifstream(filePath);
    auto mesh = Mesh{};

    auto vertexPositions = std::vector<Eigen::Vector3<Mesh::ScalarType>>{};
    auto normals = std::vector<Eigen::Vector3<Mesh::ScalarType>>{};
    auto faceNormals = std::vector<Eigen::Vector3<Mesh::ScalarType>>{};
    auto faces = std::vector<Eigen::Vector3<Mesh::VertexId>>{};

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
            std::ranges::copy(
                std::ranges::istream_view<Mesh::ScalarType>(iss),
                vertexPositions.emplace_back().begin()
            );
        } else if (rowType == "vn") {
            std::ranges::copy(
                std::ranges::istream_view<Mesh::ScalarType>(iss),
                normals.emplace_back().begin()
            );
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

            if (faceData.size() > 3) ThrowMessage("ERROR", "Non-triangle faces not supported");

            std::ranges::copy(
                faceData | std::views::elements<0>,
                faces.emplace_back().begin()
            );

            faceNormals.emplace_back(normals[std::get<1>(faceData.front())]);
        } else {
            DebugMessage("WARN", std::format("Encountered unknown row type {}", rowType));
        }
    }

    mesh.vertexPositions.resize(vertexPositions.size(), Eigen::NoChange);
    mesh.faceNormals.resize(faceNormals.size(), Eigen::NoChange);
    mesh.faces.resize(faces.size(), Eigen::NoChange);

    DebugMessage("INFO", std::format("Found {} faces and {} vertices", mesh.faces.rows(), mesh.vertexPositions.rows()));

    std::copy(
        vertexPositions.begin(),
        vertexPositions.end(),
        mesh.vertexPositions.rowwise().begin()
    );

    std::copy(
        faceNormals.begin(),
        faceNormals.end(),
        mesh.faceNormals.rowwise().begin()
    );

    std::copy(
        faces.begin(),
        faces.end(),
        mesh.faces.rowwise().begin()
    );

    return mesh;
}

MeshComponent::MeshComponent(const Mesh& mesh) {
    std::vector<VertexData> vertexData;
    vertexData.reserve(mesh.faces.size());
    
    auto faceId = 0u;
    for (auto face : mesh.faces.rowwise()) {
        for (auto vertexId : face) {
            auto vData = VertexData{};
            std::ranges::copy(mesh.vertexPositions.row(vertexId), vData.position.begin());
            std::ranges::copy(mesh.faceNormals.row(faceId), vData.normal.begin());

            vertexData.emplace_back(vData);
        }
        ++faceId;
    }

    numVertices = vertexData.size();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(VertexData), vertexData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), reinterpret_cast<void *>(offsetof(VertexData, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), reinterpret_cast<void *>(offsetof(VertexData, normal)));
    glEnableVertexAttribArray(1);
}

MeshComponent::MeshComponent(MeshComponent&& other) {
    vao = other.vao;
    vbo = other.vbo;

    other.vao = 0;
    other.vbo = 0;
}

MeshComponent::~MeshComponent() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}