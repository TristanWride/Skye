add_subdirectory(SDL EXCLUDE_FROM_ALL)

# GoogleTest dependency
add_subdirectory (googletest EXCLUDE_FROM_ALL)

# GLM Dependency
add_library (glm INTERFACE)
target_include_directories (glm INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/glm")
