add_library(skye STATIC
  src/Entity.cpp
  src/Util.cpp
)
target_include_directories(skye PUBLIC include)
target_compile_features(skye PUBLIC cxx_std_23)
