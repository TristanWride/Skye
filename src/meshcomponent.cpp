#include "meshcomponent.h"
#include "debugutils.h"

#include <algorithm>
#include <fstream>
#include <ranges>
#include <concepts>

auto Mesh::ReadObj(const char* filePath) -> Mesh {
    DebugMessage("INFO", std::format("Reading object file \"{}\"", filePath));
    auto inputFile = std::ifstream(filePath);
    auto mesh = Mesh{};

    auto vertices = std::vector<Eigen::Vector3<Mesh::ScalarType>>{};
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
                vertices.emplace_back().begin()
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

    mesh.vertices.resize(vertices.size(), Eigen::NoChange);
    mesh.faceNormals.resize(faceNormals.size(), Eigen::NoChange);
    mesh.faces.resize(faces.size(), Eigen::NoChange);

    DebugMessage("INFO", std::format("Found {} faces and {} vertices", mesh.faces.rows(), mesh.vertices.rows()));

    std::copy(
        vertices.begin(),
        vertices.end(),
        mesh.vertices.rowwise().begin()
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