# GLFW dependency
set (GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/glfw")

# GoogleTest dependency
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/googletest")

# GLM Dependency
add_library (glm INTERFACE)
target_include_directories (glm INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/glm")
