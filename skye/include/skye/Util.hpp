#pragma once

#include <format>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace skye::util {

namespace detail {

template <class... Args>
struct SrcMessageFormat {
public:
  std::format_string<Args...> fmt_;
  std::source_location        sl_;

public:
  // Intentionally implicit: converts the string literal passed to Fail() and captures the caller's location.
  template <class T>
  consteval SrcMessageFormat( const T & fmt, std::source_location sl = std::source_location::current() ) noexcept
    : fmt_{ fmt }
    , sl_{ sl } {}
};

// Adds the source location, prints to stderr and throws Failure. Defined in Util.cpp.
[[noreturn]] void RaiseFailure( const std::source_location & sl, std::string_view message );

}

struct Failure : public std::runtime_error {
  // Wraps the full failure message, including the source location.
  explicit Failure( const std::string & msg );
};

// Formats the message, prints it with the caller's location and throws Failure.
// Templates must stay in the header; only the formatting happens here.
template <class... Args>
[[noreturn]] void Fail( detail::SrcMessageFormat<std::type_identity_t<Args>...> fmt, Args &&... args ) {
  detail::RaiseFailure( fmt.sl_, std::format( fmt.fmt_, std::forward<Args>( args )... ) );
}

}
