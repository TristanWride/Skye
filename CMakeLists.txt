cmake_minimum_required (VERSION 3.27)

project (PersonalProjectKnit)

set (CMAKE_CXX_STANDARD 23)

enable_testing ()

if(MSVC)
  # Force to always compile with W4
  add_compile_options (/W4 /EHsc "$<$<CONFIG:Debug>:/Od>" "$<$<CONFIG:Debug>:/fsanitize=address>" "$<$<CONFIG:Release>:/O2>")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  add_compile_options (-Wall -pedantic)
endif()

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
