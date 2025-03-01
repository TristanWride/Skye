#include "meshcomponent.hpp"
#include "debugutils.hpp"

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
    DebugMessage("INFO", "Reading object file \"{}\"", filePath);
    auto inputFile = std::ifstream(filePath);

    if (!inputFile.is_open()) {
        DebugMessage("ERROR", "Failed to open file \"{}\"", filePath);
        return Mesh{};
    }

    return Mesh::ReadObj(inputFile);
}

auto Mesh::ReadObj(std::string_view inputStr) -> Mesh {
    auto objStream = std::stringstream{};
    objStream << inputStr;
    return Mesh::ReadObj(objStream);
}

