#ifndef WYRM_TRAITS_H
#define WYRM_TRAITS_H

#include <type_traits>

namespace wyrm {
template <typename...> struct is_one_of {
  static constexpr bool value = false;
};

template <typename F, typename S, typename... T> struct is_one_of<F, S, T...> {
  static constexpr bool value =
      std::is_same<F, S>::value || is_one_of<F, T...>::value;
};

template <typename T, typename... Ts>
constexpr bool is_one_of_v = is_one_of<T, Ts...>::value;
} // namespace wyrm

#endif
