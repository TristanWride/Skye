#include "skye/Util.hpp"

#include <iostream>
#include <print>

namespace skye::util {

Failure::Failure( const std::string & msg )
  : std::runtime_error{ msg } {}

namespace detail {

void RaiseFailure( const std::source_location & sl, std::string_view message ) {
  auto msg = std::format( "Failure at {}({},{}):{}\n{}",
                          sl.file_name(),
                          sl.line(),
                          sl.column(),
                          sl.function_name(),
                          message );
  std::println( std::cerr, "{}", msg );
  throw Failure{ msg };
}

}

}
