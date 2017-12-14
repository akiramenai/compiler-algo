/// \file
/// \brief Compatibility with old versions of the standard library.
/// Provide aliases for STL types introduced in C++17 in wyrm namespace so that
/// if the version of the standard library used to build wyrm doesn't support a
/// type boost analog is used instead.
#ifndef COMPATIBILITY_H
#define COMPATIBILITY_H

#if __has_include(<variant>) && !defined(USE_BOOST_CONTAINERS)
#include <variant>
#else
#include <boost/variant.hpp>
#endif
#include <iostream>
#if __has_include(<optional>) && !defined(USE_BOOST_CONTAINERS)
#include <optional>
#else
#include <boost/optional.hpp>
#endif
#if __has_include(<string_view>) && !defined(USE_BOOST_CONTAINERS)
#include <string_view>
#else
#include <boost/utility/string_view.hpp>
#endif

namespace wyrm {
#if __has_include(<variant>) && !defined(USE_BOOST_CONTAINERS)
template <typename... Args> using variant = std::variant<Args...>;
inline auto visit = [](auto &&... Args) {
  return std::visit(std::forward<decltype(Args)>(Args)...);
};
#else
template <typename... Args> using variant = boost::variant<Args...>;
inline auto visit = [](auto &&... Args) {
  return boost::apply_visitor(std::forward<decltype(Args)>(Args)...);
};
#endif

#if __has_include(<optional>) && !defined(USE_BOOST_CONTAINERS)
template <typename ValueT> using optional = std::optional<ValueT>;
#else
template <typename ValueT> using optional = boost::optional<ValueT>;
#endif

#if __has_include(<string_view>) && !defined(USE_BOOST_CONTAINERS)
using string_view = std::string_view;
#else
using string_view = boost::string_view;
#endif
}

#endif // COMPATIBILITY_H
