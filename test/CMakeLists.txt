macro(skye_add_test TESTNAME)
    # target names share one namespace with the consuming project, so prefix them
    set(SKYE_TEST_TARGET skye_${TESTNAME})
    # create an executable in which the tests will be stored
    add_executable(${SKYE_TEST_TARGET} ${ARGN})
    # skye::testing brings in Skye, googletest and gmock; gtest_main supplies main()
    target_link_libraries(${SKYE_TEST_TARGET} skye::testing GTest::gtest_main)
    target_compile_features (${SKYE_TEST_TARGET} PRIVATE cxx_std_23)
    # must run before gtest_discover_tests: discovery runs the executable after building
    skye_copy_asan_runtime(${SKYE_TEST_TARGET})
    # gtest_discover_tests replaces gtest_add_tests,
    # see https://cmake.org/cmake/help/v3.10/module/GoogleTest.html for more options to pass to it
    gtest_discover_tests(${SKYE_TEST_TARGET}
        # set a working directory so your project root so that you can find test data via paths relative to the project root
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        TEST_PREFIX "skye."
        PROPERTIES
            LABELS skye
            VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    )
    set_target_properties(${SKYE_TEST_TARGET} PROPERTIES FOLDER skye/tests)
endmacro()

add_subdirectory( skye )
