#pragma once

#include <span>
#include <array>
#include <ranges>
#include "ranges_utils.hpp"

template<size_t A, size_t B>
static std::array<int, A - B> array_diff(const std::array<int, A> &big, const std::array<int, B> &small) {
  std::array<int, A - B> result{};
  int n = 0;
  for (int i = 0; i < A; ++i)
    if (!std::ranges::contains(small, big[i]))
      result[n++] = big[i];
  return result;

}

template<size_t A, size_t B>
static int array_eq_count(std::array<int, A> a, const std::array<int, B> b) {
  std::ranges::sort(a);
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
