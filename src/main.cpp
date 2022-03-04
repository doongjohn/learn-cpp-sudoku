// This is a simple sudoku game
//
// - [x] valid sudoku grid generator
// - [ ] valid sudoku puzzle generator
// - [ ] customizable difficulty
// - [ ] create interactive gui
//
// about sudoku generator & solver:
// https://stackoverflow.com/questions/7135471/is-there-any-algorithm-that-can-solve-any-traditional-sudoku-puzzles-without-gu
// https://medium.com/analytics-vidhya/implement-a-sudoku-algorithm-without-backtracking-beginner-python-353de7217d4c
// https://github.com/vaithak/Sudoku-Generator
// https://github.com/t-dillon/tdoku
// https://github.com/robatron/sudoku.js
// https://github.com/ZeroX-DG/vudoku

#include <span>
#include <array>
#include <ranges>
#include <random>
#include <algorithm>
#include <iostream>

#include "array_utils.hpp"
#include "ranges_utils.hpp"

using std::array;
using std::span;
namespace ranges = std::ranges;

using SudokuGrid = array<int, 81>;
using SudokuBox = array<int, 9>;
using SudokuRow = array<SudokuBox, 3>;
using SudokuSingleRows = array<array<int, 9>, 3>;

static void apply_to(SudokuGrid &grid, const array<SudokuRow, 3> &rows) {
  for (int r = 0; r < 3; ++r)
    for (int b = 0; b < 3; ++b)
      for (int i = 0; i < 9; ++i)
        grid[r * 27 + b * 3 + i + i / 3 * 6] = rows[r][b][i];
}

static SudokuBox create_random_box(std::mt19937 &rng) {
  SudokuBox box = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  ranges::shuffle(box, rng);
  return box;
}

static SudokuRow create_random_fr_valid_row0(std::mt19937 &rng) {
  SudokuRow result{};
  array<int, 9> nums{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };

  // fill the first row with unique numbers
  ranges::shuffle(nums, rng);
  int n = 0;
  for (int r = 0; r < 3; ++r) {
    for (int i = 0; i < 3; ++i) {
      result[r][i] = nums[n++];
    }
  }

  // fill other two rows
  for (int r = 0; r < 3; ++r) {
    ranges::shuffle(nums, rng);
    n = 0;
    for (int i = 3; i < 9; ++n) {
      if (result[r][0] != nums[n] && result[r][1] != nums[n] && result[r][2] != nums[n])
        result[r][i++] = nums[n];
    }
  }

  return result;
}

static SudokuRow create_random_fr_valid_row1(std::mt19937 &rng, const SudokuRow row0) {
  SudokuRow result{};

  // fill the first row with random unique numbers
  array<int, 9> nums{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  ranges::shuffle(nums, rng);
  for (int r = 0; r < 3; ++r) {
    for (int c = 0; c < 3; ++c) {
      const array<int, 3> col2{ row0[r][c], row0[r][c + 3], row0[r][c + 6] };
      for (int i = 0; i < 9; ++i) {
        if (nums[i] != 0 && !ranges::contains(col2, nums[i])) {
          result[r][c] = nums[i];
          nums[i] = 0;
          break;
        }
      }
    }
  }

  ranges::sort(nums, std::ranges::greater());
  for (int lr = 0; lr < 3; ++lr) {
    if (result[2][lr] != 0) continue;
    const array<int, 3> upcol{ row0[2][lr], row0[2][lr + 3], row0[2][lr + 6] };
    for (int r = 0; r < 3; ++r) {
      for (int c = 0; c < 3; ++c) {
        for (int i = 0; i < 9; ++i) {
          const array<int, 3> col2{ row0[r][c], row0[r][c + 3], row0[r][c + 6] };
          if (!ranges::contains(col2, nums[0]) && !ranges::contains(upcol, result[r][c])) {
            result[2][lr] = result[r][c];
            result[r][c] = nums[0];
            goto DONE;
          }
        }
      }
    }
  }
  DONE:;

  // fill other two rows
  nums = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  array<int, 6> available{};
  for (int r = 0; r < 3; ++r) {
    available = {
      result[(r + 1) % 3][0],
      result[(r + 1) % 3][1],
      result[(r + 1) % 3][2],
      result[(r + 2) % 3][0],
      result[(r + 2) % 3][1],
      result[(r + 2) % 3][2]
    };
    for (int c = 0; c < 3; ++c) {
      const array<int, 3> upcol{ row0[r][c], row0[r][c + 3], row0[r][c + 6] };
      for (int i = 1; i < 3; ++i) {
        ranges::shuffle(available, rng);
        for (int &n : available) {
          if (n == 0 || ranges::contains(upcol, n))
            continue;
          result[r][c + 3 * i] = n;
          n = 0;
          break;
        }
      }
    }
  }

  // 이미 정해진 첫번째 행을 제외하고 다른 숫자와 바꾼다.
  for (int r = 0; r < 3; ++r) {
    for (int c = 0; c < 3; ++c) {
      for (int rn = 1; rn < 3; ++rn) {
        if (result[r][c + 3 * rn] != 0) continue;
        int num;
        for (int n : nums) {
          if (!ranges::contains(result[r], n)) {
            num = n;
            break;
          }
        }
        const array<int, 3> upcol{ row0[r][c], row0[r][c + 3], row0[r][c + 6] };
        for (int i = 1; i < 3; ++i) {
          if (!ranges::contains(upcol, result[r][(c + i) % 3 + 3])) {
            result[r][c + 3 * rn] = result[r][(c + i) % 3 + 3];
            result[r][(c + i) % 3 + 3] = num;
            break;
          }
        }
      }
    }
  }

  return result;
}

static SudokuRow create_random_fr_valid_row2(std::mt19937 &rng, const SudokuRow row0, const SudokuRow row1) {
  SudokuRow result{};

  const array<int, 9> nums{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  array<array<int, 3>, 9> cols{
    array_diff<9, 6>(nums, { row0[0][0], row0[0][3], row0[0][6], row1[0][0], row1[0][3], row1[0][6] }),
    array_diff<9, 6>(nums, { row0[0][1], row0[0][4], row0[0][7], row1[0][1], row1[0][4], row1[0][7] }),
    array_diff<9, 6>(nums, { row0[0][2], row0[0][5], row0[0][8], row1[0][2], row1[0][5], row1[0][8] }),
    array_diff<9, 6>(nums, { row0[1][0], row0[1][3], row0[1][6], row1[1][0], row1[1][3], row1[1][6] }),
    array_diff<9, 6>(nums, { row0[1][1], row0[1][4], row0[1][7], row1[1][1], row1[1][4], row1[1][7] }),
    array_diff<9, 6>(nums, { row0[1][2], row0[1][5], row0[1][8], row1[1][2], row1[1][5], row1[1][8] }),
    array_diff<9, 6>(nums, { row0[2][0], row0[2][3], row0[2][6], row1[2][0], row1[2][3], row1[2][6] }),
    array_diff<9, 6>(nums, { row0[2][1], row0[2][4], row0[2][7], row1[2][1], row1[2][4], row1[2][7] }),
    array_diff<9, 6>(nums, { row0[2][2], row0[2][5], row0[2][8], row1[2][2], row1[2][5], row1[2][8] })
  };

  // TODO: review this algorithm
  // I don't know how this is working...
  array<int, 9> fr{
    cols[0][0],
    cols[1][0],
    cols[2][0],
    cols[3][0],
    cols[4][0],
    cols[5][0],
    cols[6][0],
    cols[7][0],
    cols[8][0]
  };
  int p = 0;
  while (true) {
    for (int i = 0; i < 9; ++i) {
      int num = fr[i];
      fr[i] = 0;
      if (ranges::contains(fr, num)) {
        cols[i][0] = cols[i][1 + p];
        cols[i][1 + p] = num;
        p = p == 0 ? 1 : 0;
      }
      fr[i] = cols[i][0];
    }
    if (array_eq_count(fr, nums) == 9) {
      for (int i = 0; i < 9; ++i) {
        result[i / 3][i % 3] = cols[i][0];
        result[i / 3][i % 3 + 3] = cols[i][1];
        result[i / 3][i % 3 + 6] = cols[i][2];
      }
      break;
    }
  }

  return result;
}

static SudokuSingleRows to_single_rows(const SudokuRow &row) {
  return {{
    {{ row[0][0], row[0][1], row[0][2], row[1][0], row[1][1], row[1][2], row[2][0], row[2][1], row[2][2] }},
    {{ row[0][3], row[0][4], row[0][5], row[1][3], row[1][4], row[1][5], row[2][3], row[2][4], row[2][5] }},
    {{ row[0][6], row[0][7], row[0][8], row[1][6], row[1][7], row[1][8], row[2][6], row[2][7], row[2][8] }}
  }};
}

static void apply_to(SudokuRow &row, const SudokuSingleRows &sr) {
  for (int i = 0; i < 3; ++i) {
    row[0][i] = sr[0][i];
    row[0][i + 3] = sr[1][i];
    row[0][i + 6] = sr[2][i];
    row[1][i] = sr[0][i + 3];
    row[1][i + 3] = sr[1][i + 3];
    row[1][i + 6] = sr[2][i + 3];
    row[2][i] = sr[0][i + 6];
    row[2][i + 3] = sr[1][i + 6];
    row[2][i + 6] = sr[2][i + 6];
  }
}

static void make_valid_sr(SudokuSingleRows &sr) {
  // TODO: review this code
  std::cout << "-> running the algorithm... (sr[1], sr[2])\n";
  int lastpos = 0;
  for (int i = 0; i < 9;) {
    // find overlapping
    if (ranges::count(sr[1], sr[1][i]) == 2) {
      // get overlapping index before actually swapping
      int overlap_index = ranges::index_of(sr[1], sr[2][i]);
      int num = sr[1][i];
      sr[1][i] = sr[2][i];
      sr[2][i] = num;
      // follow the overlapping index
      if (overlap_index != -1) {
        i = overlap_index;
      } else {
        ++lastpos;
        i = lastpos;
      }
    } else {
      ++lastpos;
      i = lastpos;
    }
  }

  // TEMP: result validation
  array<int, 9> all_sorted = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  array<int, 9> sr1_sorted = sr[1];
  array<int, 9> sr2_sorted = sr[2];
  ranges::sort(sr1_sorted);
  ranges::sort(sr2_sorted);
  if (all_sorted == sr1_sorted && all_sorted == sr2_sorted)
    std::cout << "   -> valid\n";
  else
    std::cout << "   -> invalid\n";
}

static void make_valid(SudokuRow &row) {
  SudokuSingleRows sr = to_single_rows(row);
  make_valid_sr(sr);
  apply_to(row, sr);
}

static void print(const SudokuGrid &grid) {
  for (int i = 0; i < 81; ++i) {
    if (i % 27 == 0)
      std::cout << "      ⁙───────⁙───────⁙───────⁙\n";
    if (i % 9 == 0)
      std::cout << "sr[" << (i / 9 % 3) << "] ";
    if (i % 3 == 0)
      std::cout << "│ ";
    std::cout << grid[i] << ' ';
    if ((i + 1) % 9 == 0)
      std::cout << "│\n";
  }
  std::cout << "      ⁙───────⁙───────⁙───────⁙\n";
}

int main() {
  // init random
  std::random_device rd{};
  std::mt19937 rng{rd()};

  // init sudoku grid
  array<int, 81> grid{};
  grid.fill(0);

  // this algorithm randomly fills the row of 3x3 boxes in a sudoku game with 1 to 9,
  // with the first row (of three) also filling that condition.
  SudokuRow row0 = create_random_fr_valid_row0(rng);
  SudokuRow row1 = create_random_fr_valid_row1(rng, row0);
  SudokuRow row2 = create_random_fr_valid_row2(rng, row0, row1);

  // this algorithm fills each row with 1 to 9
  // (but it skips the first row because it is already filled with 1 to 9 by the previous step)
  make_valid(row0);
  make_valid(row1);
  make_valid(row2);

  // apply sudoku rows to the grid array
  apply_to(grid, { row0, row1, row2 });
  print(grid);

  return 0;
}
