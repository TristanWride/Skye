cmake_minimum_required (VERSION 3.28)

project (PersonalProjectKnit)

set (CMAKE_CXX_STANDARD 23)

# GLFW dependency
set (GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glfw-3.3.9")

# Glad Dependency
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad")

# Eigen Dependency
add_library (eigen INTERFACE)
target_include_directories (eigen INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/eigen")

# Executable
add_executable (visualizer 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/meshcomponent.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/window.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/shader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ecsmanager.cpp"
)

target_include_directories (visualizer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries (visualizer glfw)
target_link_libraries (visualizer glad)
target_link_libraries (visualizer eigen)

target_compile_definitions (visualizer PUBLIC DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
target_compile_definitions (visualizer PUBLIC SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shader/")