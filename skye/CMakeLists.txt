add_library(skye STATIC
  src/Component.cpp
  src/Entity.cpp
  src/Event.cpp
  src/SparseSet.cpp
  src/Util.cpp
)
add_library(skye::skye ALIAS skye)
target_include_directories(skye PUBLIC include)
target_compile_features(skye PUBLIC cxx_std_23)

if (SKYE_BUILD_TESTING)
  # Games link this to get Skye, googletest and gmock (and, later, Skye's test helpers).
  # Code still includes the googletest headers it uses directly.
  add_library(skye_testing INTERFACE)
  add_library(skye::testing ALIAS skye_testing)
  target_link_libraries(skye_testing INTERFACE skye::skye GTest::gtest GTest::gmock)
endif ()
