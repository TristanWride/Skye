cmake_minimum_required (VERSION 4.4)

project (Skye)
enable_language (CXX)

enable_testing()
include (GoogleTest)
add_subdirectory (external)

add_subdirectory (examples)