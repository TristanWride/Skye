add_executable (unittest
    "ECSTest.cpp"
    "CameraComponentTest.cpp"
)

target_link_libraries (unittest gtest_main gmock vislib)
add_test (NAME unittest COMMAND unittest)