cmake_minimum_required (VERSION 3.27)

project (PersonalProjectKnit)

set (CMAKE_CXX_STANDARD 23)

enable_testing ()

if(MSVC)
  # Force to always compile with W4
  add_compile_options (/W4 /WX /EHsc "$<$<CONFIG:Debug>:/Od>" "$<$<CONFIG:Release>:/O2>")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  add_compile_options (-Wall -pedantic)
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

# GoogleTest Dependency
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/dependencies/googletest-1.14.0")

# Library
add_library (vislib STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/meshcomponent.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/window.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/shader.cpp"
)

target_include_directories (vislib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries (vislib 
    PUBLIC glfw 
    PUBLIC glad 
    PUBLIC glm
)

target_compile_definitions (vislib PUBLIC DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/")
target_compile_definitions (vislib PUBLIC SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shader/")

# Executable
add_executable (visualizer    
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)

target_link_libraries (visualizer PUBLIC vislib)

# Unittests
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/unittest")