macro(add_example name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} PRIVATE SDL3::SDL3)
  target_compile_features(${name} PRIVATE cxx_std_23)
  skye_copy_asan_runtime(${name})
endmacro()

add_example(clear)