cmake_minimum_required (VERSION 3.27)

project (PersonalProjectKnit)

set (CMAKE_CXX_STANDARD 23)

if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /MD")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

# GLFW dependency
set (GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set (GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glfw-3.3.9")

# Glad Dependency
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad")

# GLM Dependency
add_library (glm INTERFACE)
target_include_directories (glm INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm")

# Executable
add_executable (visualizer)

target_sources (visualizer PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/meshcomponent.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/window.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/shader.cpp"
)

target_include_directories (visualizer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries (visualizer glfw)
target_link_libraries (visualizer glad)
target_link_libraries (visualizer glm)

target_compile_definitions (visualizer PUBLIC DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
target_compile_definitions (visualizer PUBLIC SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shader/")