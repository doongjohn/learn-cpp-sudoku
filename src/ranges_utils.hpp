#pragma once

#include <algorithm>
#include <ranges>

namespace std { namespace ranges {

template<ranges::input_range R, class T, class Proj = std::identity>
requires std::indirect_binary_predicate<ranges::equal_to,
                                        std::projected<ranges::iterator_t<R>, Proj>,
                                        const T*>
constexpr bool contains(R&& r, size_t first, size_t last, const T& value, Proj proj = {}) {
  if (first > last || last < 0) return false;
  return ranges::find(r.begin() + first, r.begin() + last, value, proj) != r.end();
}

template<ranges::input_range R, class T, class Proj = std::identity>
requires std::indirect_binary_predicate<ranges::equal_to,
                                        std::projected<ranges::iterator_t<R>, Proj>,
                                        const T*>
constexpr bool contains(R&& r, const T& value, Proj proj = {}) {
  return ranges::find(r, value, proj) != r.end();
}

template<ranges::input_range R, class T, class Proj = std::identity>
requires std::indirect_binary_predicate<ranges::equal_to,
                                        std::projected<ranges::iterator_t<R>, Proj>,
                                        const T*>
constexpr size_t index_of(R&& r, size_t first, size_t last, const T& value, Proj proj = {}) {
  if (first > last || last < 0) return false;
  const auto it = ranges::find(r.begin() + first, r.begin() + last, value, proj);
  return it == r.end() ? -1 : it - r.begin();
}

template<ranges::input_range R, class T, class Proj = std::identity>
requires std::indirect_binary_predicate<ranges::equal_to,
                                        std::projected<ranges::iterator_t<R>, Proj>,
                                        const T*>
constexpr size_t index_of(R&& r, const T& value, Proj proj = {}) {
  const auto it = ranges::find(r, value, proj);
  return it == r.end() ? -1 : it - r.begin();
}

}}
