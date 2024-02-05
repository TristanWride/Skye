#include "debugutils.h"
#include "mesh.h"

auto main() -> int {
    auto mesh = Mesh::ReadObj(SOURCEDIR "/data/tris.obj");

    return EXIT_SUCCESS;
}