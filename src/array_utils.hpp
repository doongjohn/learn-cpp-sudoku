#pragma once

#include <array>
#include <ranges>
#include "ranges_utils.hpp"

template<size_t A, size_t B>
inline
static std::array<int, A - B> array_diff(const std::array<int, A> &big, const std::array<int, B> &small) {
  std::array<int, A - B> result{};
  for (int i = 0, n = 0; i < A; ++i)
    if (!std::ranges::contains(small, big[i]))
      result[n++] = big[i];
  return result;
}

template<size_t A, size_t B>
static int array_eq_count_sorted(const std::array<int, A> &a, const std::array<int, B> &b) {
  // this function assumes that the argument `a : array` is sorted
  int count = 0;
  int prev = a[0];
  for (int i = 0; i < A; ++i) {
    if (i != 0 && prev == a[i]) {
      prev = a[i];
      continue;
    }
    prev = a[i];
    if (std::ranges::contains(b, a[i]))
      ++count;
  }
  return count;
}

template<size_t A, size_t B>
inline
static int array_eq_count(std::array<int, A> a, std::array<int, B> b) {
  std::ranges::sort(a);
  return array_eq_count_sorted(a, b);
}
