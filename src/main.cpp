#include "mesh.h"

#include <iostream>
#include <iomanip>

auto main() -> int {
    auto mesh = Mesh::ReadObj(SOURCEDIR "/data/tris.obj");

    return EXIT_SUCCESS;
}