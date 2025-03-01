macro (package_add_test TESTNAME)
    add_executable (${TESTNAME} ${ARGN})

    target_link_libraries (
        ${TESTNAME} 
        PUBLIC gtest 
        PUBLIC gmock 
        PUBLIC gtest_main
    )

    gtest_discover_tests (${TESTNAME}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    )
    set_target_properties (${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

include (GoogleTest)

package_add_test (unittest
    "ECSTest.cpp"
    "CameraComponentTest.cpp"
    "MeshTest.cpp"
    "ComponentManagerTest.cpp"
)

target_link_libraries (unittest PUBLIC vislib)
target_include_directories (unittest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

