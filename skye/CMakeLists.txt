add_library(skye STATIC
  src/Entity.cpp
  src/Event.cpp
  src/Util.cpp
)
add_library(skye::skye ALIAS skye)
target_include_directories(skye PUBLIC include)
target_compile_features(skye PUBLIC cxx_std_23)

if (SKYE_BUILD_TESTING)
  # Header-only for now; games link this to get googletest, gmock and Skye's test helpers.
  add_library(skye_testing INTERFACE)
  add_library(skye::testing ALIAS skye_testing)
  target_link_libraries(skye_testing INTERFACE skye::skye GTest::gtest GTest::gmock)
endif ()
