cmake_minimum_required (VERSION 4.4)

project (Skye)
enable_language (CXX)

option (SKYE_BUILD_TESTS "Build the Skye unit tests" ${PROJECT_IS_TOP_LEVEL})
option (SKYE_BUILD_EXAMPLES "Build the Skye examples" ${PROJECT_IS_TOP_LEVEL})

if (PROJECT_IS_TOP_LEVEL)
  include (CTest)
elseif (SKYE_BUILD_TESTS)
  # Registers Skye's tests. The consuming project must also call enable_testing() or
  # include(CTest) in its own root for ctest to find them.
  enable_testing ()
endif ()

if (SKYE_BUILD_TESTS)
  include (GoogleTest)
endif ()

# MSVC links ASan builds against clang_rt.asan_dynamic-x86_64.dll, which lives next to cl.exe
# and is not on PATH. Copy it beside the executable in the Asan configuration so the program
# (and gtest's post-build test discovery) can start. Call this before gtest_discover_tests().
function (skye_copy_asan_runtime target)
  if (NOT MSVC)
    return ()
  endif ()
  get_filename_component (MSVC_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
  set (ASAN_RUNTIME "${MSVC_BIN_DIR}/clang_rt.asan_dynamic-x86_64.dll")
  add_custom_command (TARGET ${target} POST_BUILD
    COMMAND "$<$<CONFIG:Asan>:${CMAKE_COMMAND};-E;copy_if_different;${ASAN_RUNTIME};$<TARGET_FILE_DIR:${target}>>"
    COMMAND_EXPAND_LISTS
  )
endfunction ()

add_subdirectory (external)

if (SKYE_BUILD_EXAMPLES)
  add_subdirectory (examples)
endif ()
add_subdirectory (skye)
if (SKYE_BUILD_TESTS)
  add_subdirectory (test)
endif ()