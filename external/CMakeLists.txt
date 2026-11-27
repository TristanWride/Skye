# Skipped if the consuming project already provides googletest. Its add_subdirectory build
# defines the GTest::gtest, GTest::gmock, GTest::gtest_main and GTest::gmock_main aliases.
if (SKYE_BUILD_TESTING AND NOT TARGET GTest::gtest)
  # Use the DLL C runtime (/MD) like the rest of the project, otherwise MSVC fails to link (LNK2038).
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  add_subdirectory (googletest EXCLUDE_FROM_ALL)

  mark_as_advanced(
    BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
    gmock_build_tests gtest_build_samples gtest_build_tests
    gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols
  )

  set_target_properties(gtest PROPERTIES FOLDER extern)
  set_target_properties(gtest_main PROPERTIES FOLDER extern)
  set_target_properties(gmock PROPERTIES FOLDER extern)
  set_target_properties(gmock_main PROPERTIES FOLDER extern)
endif ()

if (NOT TARGET SDL3::SDL3)
  add_subdirectory(SDL EXCLUDE_FROM_ALL)
endif ()
