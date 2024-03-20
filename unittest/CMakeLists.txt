add_executable (unittest
    "ECSTest.cpp"
    "CameraComponentTest.cpp"
    "MeshTest.cpp"
)

target_link_libraries (unittest gtest_main gmock vislib)
target_include_directories (unittest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_test (NAME unittest COMMAND unittest)