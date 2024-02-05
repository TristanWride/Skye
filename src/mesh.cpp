#include "mesh.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <format>
#include <algorithm>
#include <iterator>
#include <tuple>
#include <ranges>

auto Mesh::ReadObj(const std::string& filePath) -> Mesh {
    auto inputFile = std::ifstream(filePath);
    auto vertexPositions = std::vector<Eigen::Vector3d>{};
    auto normals = std::vector<Eigen::Vector3d>{};
    auto faceNormals = std::vector<Eigen::Vector3d>{};
    auto faces = std::vector<std::vector<VertexId>>{};

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
            std::copy(
                std::istream_iterator<double>(iss),
                std::istream_iterator<double>(),
                vertexPositions.emplace_back().begin()
            );
        } else if (rowType == "vn") {
            std::copy(
                std::istream_iterator<double>(iss),
                std::istream_iterator<double>(),
                normals.emplace_back().begin()
            );
        } else if (rowType == "f") {
            auto readFaceVertices = [](const std::string& entry) 
                    -> std::tuple<VertexId, VertexId> {
                const auto ed = std::find(entry.cbegin(), entry.cend(), '/');
                auto vIx = std::stoul(std::string(entry.cbegin(), ed)) - 1;
                const auto bg = std::find_if(
                    ed, entry.cend(), 
                    [](auto c){ return c != '/'; }
                );
                auto vnIx = std::stoul(std::string(bg, entry.cend())) - 1;
                return {vIx, vnIx};
            };

            auto faceData = std::vector<std::tuple<VertexId, VertexId>>{};
            std::transform(
                std::istream_iterator<std::string>(iss),
                std::istream_iterator<std::string>(),
                std::back_inserter(faceData),
                readFaceVertices
            );
            std::ranges::copy(
                faceData | std::views::elements<0>,
                std::back_inserter(faces.emplace_back())
            );
            const auto& normal = normals[std::get<1>(faceData[0])];
            std::ranges::copy(
                normal,
                faceNormals.emplace_back().begin()
            );
        } else {
            std::cout 
                << "[WARN]: Encountered unknown row type " 
                << rowType << '\n';
        }
    }

    auto mesh = Mesh{};

    const auto numVertices = vertexPositions.size();
    const auto numFaces = faces.size();

    auto edgeTriplets 
        = std::vector<Eigen::Triplet<Orientation>>{};
    auto faceTriplets 
        = std::vector<Eigen::Triplet<Orientation>>{};

    auto edgeId = 0;
    for (auto [faceId, face] : faces | std::views::enumerate) {
        face.emplace_back(face.front());
        for (auto edge : face | std::views::adjacent<2>) {
            auto from = std::get<0>(edge);
            auto to = std::get<1>(edge);
            if (from == to) std::cout 
                << "[WARN]: Reflexive edge detected at face " << faceId << ": " 
                << from << " -> " << to << '\n';
            auto edgeFaceOrientation = 1;
            if (from > to) {
                std::swap(from, to);
                edgeFaceOrientation = -1;
            }
            edgeTriplets.emplace_back(edgeId, from, -1);
            edgeTriplets.emplace_back(edgeId, to, 1);
            faceTriplets.emplace_back(faceId, edgeId, edgeFaceOrientation);

            ++edgeId;
        }
    }

    const auto numEdges = edgeId;

    mesh.edges.resize(numEdges, numVertices);
    mesh.faces.resize(numFaces, numEdges);

    mesh.edges.setFromTriplets(
        edgeTriplets.begin(), 
        edgeTriplets.end(), 
        [](auto&& a, auto&& b) { return std::forward<decltype(b)>(b); }
    );
    mesh.faces.setFromTriplets(
        faceTriplets.begin(), 
        faceTriplets.end(), 
        [](auto&& a, auto&& b) { return std::forward<decltype(b)>(b); }
    );

    mesh.vertexPositions.resize(vertexPositions.size(), 3);
    std::copy(
        vertexPositions.begin(), vertexPositions.end(),
        mesh.vertexPositions.rowwise().begin()
    );

    mesh.faceNormals.resize(faceNormals.size(), 3);
    std::copy(
        faceNormals.begin(), faceNormals.end(),
        mesh.faceNormals.rowwise().begin()
    );

    return mesh;
}