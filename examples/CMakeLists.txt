macro(skye_add_example name)
  # target names share one namespace with the consuming project, so prefix them
  add_executable(skye_example_${name} ${name}.cpp)
  target_link_libraries(skye_example_${name} PRIVATE SDL3::SDL3)
  target_compile_features(skye_example_${name} PRIVATE cxx_std_23)
  skye_copy_asan_runtime(skye_example_${name})
  set_target_properties(skye_example_${name} PROPERTIES FOLDER skye/examples)
endmacro()

skye_add_example(clear)
