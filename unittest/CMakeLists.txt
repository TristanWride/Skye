add_executable (unittest
    "${CMAKE_CURRENT_SOURCE_DIR}/ECSTest.cpp"
)

target_link_libraries (unittest gtest_main gmock vislib)
add_test (NAME unittest COMMAND unittest)