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

#include <array>
#include <ranges>
#include <random>
#include <algorithm>
#include <iostream>

#include "array_utils.hpp"
#include "ranges_utils.hpp"

using std::array;
namespace ranges = std::ranges;

using SudokuGrid = array<int, 81>;
using SudokuBox = array<int, 9>;
using SudokuRow = array<SudokuBox, 3>;
using SudokuRowRows = array<array<int, 9>, 3>;

static void apply_to(SudokuGrid &grid, const array<SudokuRow, 3> &rows) {
  for (int r = 0; r < 3; ++r)
    for (int b = 0; b < 3; ++b)
      for (int i = 0; i < 9; ++i)
        grid[r * 27 + b * 3 + i + i / 3 * 6] = rows[r][b][i];
}

static SudokuRow create_random_fr_valid_row0(std::mt19937 &rng) {
  SudokuRow result = {};
  array<int, 9> nums{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };

  // fill the first row with 1 to 9
  ranges::shuffle(nums, rng);
  for (int b = 0, n = 0; b < 3; ++b) {
    result[b][0] = nums[n++];
    result[b][1] = nums[n++];
    result[b][2] = nums[n++];
  }

  // fill other two rows
  for (int b = 0; b < 3; ++b) {
    ranges::shuffle(nums, rng);
    for (int i = 3, n = 0; i < 9;) {
      if (result[b][0] != nums[n] && result[b][1] != nums[n] && result[b][2] != nums[n])
        result[b][i++] = nums[n];
      ++n;
    }
  }

  return result;
}

static SudokuRow create_random_fr_valid_row1(std::mt19937 &rng, const SudokuRow row0) {
  SudokuRow result{};
  array<int, 9> nums{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };

  // fill the first row with 1 to 9
  ranges::shuffle(nums, rng);
  for (int b = 0; b < 3; ++b) {
    for (int c = 0; c < 3; ++c) {
      for (int i = 0; i < 9; ++i) {
        if (
          // check zero
          nums[i] != 0 &&
          // check upper column
          row0[b][c] != nums[i] &&
          row0[b][c + 3] != nums[i] &&
          row0[b][c + 6] != nums[i]
        ) {
          result[b][c] = nums[i];
          nums[i] = 0;
          break;
        }
      }
    }
  }
  // fill an empty cell in the last box
  for (int lb = 0; lb < 3; ++lb) {
    // skip if not empty
    if (result[2][lb] != 0)
      continue;
    // get the missing number
    int num = 0;
    for (int n : nums) if (n != 0) { num = n; break; }
    // resolve the missing number
    for (int b = 0; b < 3; ++b) {
      for (int c = 0; c < 3; ++c) {
        for (int i = 0; i < 9; ++i) {
          if (
            // check upper column of the epmty cell
            row0[2][lb] != result[b][c] &&
            row0[2][lb + 3] != result[b][c] &&
            row0[2][lb + 6] != result[b][c] &&
            // check upper column of the target cell
            row0[b][c] != num &&
            row0[b][c + 3] != num &&
            row0[b][c + 6] != num
          ) {
            // put the target cells number in the empty cell
            // and put the missing number in the target cell
            result[2][lb] = result[b][c];
            result[b][c] = num;
            // exit the loop because there can be only one empty cell
            goto LOOP_EXIT;
          }
        }
      }
    }
  }
  LOOP_EXIT:;

  // fill other two rows
  for (int b = 0; b < 3; ++b) {
    nums = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    // exclude already used numbers in this box
    for (int n : result[b]) if (n != 0) nums[n - 1] = 0;
    ranges::shuffle(nums, rng);
    for (int i = 3; i < 9; ++i) {
      for (int &n : nums) {
        if (
          // check zero
          n != 0 &&
          // check upper column
          row0[b][i % 3] != n &&
          row0[b][i % 3 + 3] != n &&
          row0[b][i % 3 + 6] != n
        ) {
          result[b][i] = n;
          n = 0;
          break;
        }
      }
    }
  }
  // find and fill the empty cell
  for (int b = 0; b < 3; ++b) {
    for (int bn = 6; bn < 9; ++bn) {
      // skip if not empty
      if (result[b][bn] != 0)
        continue;
      // get the missing number in this box
      int num;
      for (int n = 1; n < 10; ++n) {
        if (!ranges::contains(result[b], n)) {
          num = n;
          break;
        }
      }
      // resolve the missing number
      // (skip the first row because its value has been fixed by the previous step)
      for (int i = 3; i < 9; ++i) {
        if (
          // check self or upper cell
          i != bn && i != bn - 3 &&
          // check upper column
          row0[b][bn % 3] != result[b][i] &&
          row0[b][bn % 3 + 3] != result[b][i] &&
          row0[b][bn % 3 + 6] != result[b][i]
        ) {
          // put the target cells number in the empty cell
          // and put the missing number in the target cell
          result[b][bn] = result[b][i];
          result[b][i] = num;
          break;
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
  // I don't know how this is working... it may loop infinitely...
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
      int num = cols[i][0];
      fr[i] = 0;
      if (ranges::contains(fr, num)) {
        cols[i][0] = cols[i][1 + p];
        cols[i][1 + p] = num;
      }
      fr[i] = cols[i][0];
    }
    p ^= 1;
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

static SudokuRowRows split_rows(const SudokuRow &row) {
  return {{
    {{ row[0][0], row[0][1], row[0][2], row[1][0], row[1][1], row[1][2], row[2][0], row[2][1], row[2][2] }},
    {{ row[0][3], row[0][4], row[0][5], row[1][3], row[1][4], row[1][5], row[2][3], row[2][4], row[2][5] }},
    {{ row[0][6], row[0][7], row[0][8], row[1][6], row[1][7], row[1][8], row[2][6], row[2][7], row[2][8] }}
  }};
}

static void apply_to(SudokuRow &row, const SudokuRowRows &rr) {
  for (int i = 0; i < 3; ++i) {
    row[0][i] = rr[0][i];
    row[0][i + 3] = rr[1][i];
    row[0][i + 6] = rr[2][i];
    row[1][i] = rr[0][i + 3];
    row[1][i + 3] = rr[1][i + 3];
    row[1][i + 6] = rr[2][i + 3];
    row[2][i] = rr[0][i + 6];
    row[2][i + 3] = rr[1][i + 6];
    row[2][i + 6] = rr[2][i + 6];
  }
}

static void make_valid_rr(SudokuRowRows &rr) {
  std::cout << "-> running the algorithm... (sr[1], sr[2])\n";
  int lastpos = 0;
  for (int i = 0; i < 9;) {
    // find overlapping
    if (ranges::count(rr[1], rr[1][i]) == 2) {
      // get overlapping index before actually swapping
      int overlap_index = ranges::index_of(rr[1], rr[2][i]);
      int num = rr[1][i];
      rr[1][i] = rr[2][i];
      rr[2][i] = num;
      // follow the overlapping index
      if (overlap_index != -1) {
        i = overlap_index;
        lastpos = -1;
      } else {
        i = ++lastpos;
      }
    } else {
      i = ++lastpos;
    }
  }

  // TEMP: result validation
  array<int, 9> all_sorted = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  array<int, 9> rr1_sorted = rr[1];
  array<int, 9> rr2_sorted = rr[2];
  ranges::sort(rr1_sorted);
  ranges::sort(rr2_sorted);
  if (all_sorted == rr1_sorted && all_sorted == rr2_sorted)
    std::cout << "   -> valid\n";
  else
    std::cout << "   -> invalid\n";
}

static void make_valid(SudokuRow &row) {
  SudokuRowRows rr = split_rows(row);
  make_valid_rr(rr);
  apply_to(row, rr);
}

static bool check_valid(SudokuGrid grid) {
  // check if the given sudoku grid is valid
  const array<int, 9> nums{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  // check rows
  std::cout << "checking grid rows...\n";
  for (int i = 0; i < 9; ++i) {
    int count = array_eq_count_sorted(
        nums, array{
          grid[9 * i],
          grid[9 * i + 1],
          grid[9 * i + 2],
          grid[9 * i + 3],
          grid[9 * i + 4],
          grid[9 * i + 5],
          grid[9 * i + 6],
          grid[9 * i + 7],
          grid[9 * i + 8]
        }
      );
    std::cout
      << grid[9 * i] << ' '
      << grid[9 * i + 1] << ' '
      << grid[9 * i + 2] << ' '
      << grid[9 * i + 3] << ' '
      << grid[9 * i + 4] << ' '
      << grid[9 * i + 5] << ' '
      << grid[9 * i + 6] << ' '
      << grid[9 * i + 7] << ' '
      << grid[9 * i + 8] << '\n';
    if (count != 9) {
      std::cout << "invalid at " << i << '\n';
      return false;
    }
  }
  // check columns
  std::cout << "checking grid columns...\n";
  for (int i = 0; i < 9; ++i) {
    int count = array_eq_count_sorted(
        nums, array{
          grid[i],
          grid[i + 9],
          grid[i + 18],
          grid[i + 27],
          grid[i + 36],
          grid[i + 45],
          grid[i + 54],
          grid[i + 63],
          grid[i + 72]
        }
      );
    std::cout
      << grid[i] << ' '
      << grid[i + 9] << ' '
      << grid[i + 18] << ' '
      << grid[i + 27] << ' '
      << grid[i + 36] << ' '
      << grid[i + 45] << ' '
      << grid[i + 54] << ' '
      << grid[i + 63] << ' '
      << grid[i + 72] << '\n';
    if (count != 9) {
      std::cout << "invalid at " << i << '\n';
      return false;
    }
  }
  // check boxes
  std::cout << "checking grid boxes...\n";
  for (int i = 0; i < 9; ++i) {
    int count = array_eq_count_sorted(
        nums, array{
          grid[i * 3 + i / 3 * 18],
          grid[i * 3 + i / 3 * 18 + 1],
          grid[i * 3 + i / 3 * 18 + 2],
          grid[i * 3 + i / 3 * 18 + 9],
          grid[i * 3 + i / 3 * 18 + 10],
          grid[i * 3 + i / 3 * 18 + 11],
          grid[i * 3 + i / 3 * 18 + 18],
          grid[i * 3 + i / 3 * 18 + 19],
          grid[i * 3 + i / 3 * 18 + 20]
        }
      );
    std::cout
      << grid[i * 3 + i / 3 * 18] << ' '
      << grid[i * 3 + i / 3 * 18 + 1] << ' '
      << grid[i * 3 + i / 3 * 18 + 2] << ' '
      << grid[i * 3 + i / 3 * 18 + 9] << ' '
      << grid[i * 3 + i / 3 * 18 + 10] << ' '
      << grid[i * 3 + i / 3 * 18 + 11] << ' '
      << grid[i * 3 + i / 3 * 18 + 18] << ' '
      << grid[i * 3 + i / 3 * 18 + 19] << ' '
      << grid[i * 3 + i / 3 * 18 + 20] << '\n';
    if (count != 9) {
      std::cout << "invalid at " << i << '\n';
      return false;
    }
  }
  return true;
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
  for (int i = 0; i < 1000; ++i) {
    // init random
    std::random_device rd{};
    std::mt19937 rng{rd()};

    // init sudoku grid
    array<int, 81> grid{};

    // this algorithm randomly fills the row of 3x3 boxes in a sudoku game with 1 to 9,
    // with the first row (of three) also filling that condition.
    std::cout << "create random sudoku row\n";
    SudokuRow row0 = create_random_fr_valid_row0(rng);
    SudokuRow row1 = create_random_fr_valid_row1(rng, row0);
    SudokuRow row2 = create_random_fr_valid_row2(rng, row0, row1);

    // this algorithm fills each row with 1 to 9
    // (but it skips the first row because it is already filled with 1 to 9 by the previous step)
    std::cout << "make sudoku row valid\n";
    make_valid(row0);
    make_valid(row1);
    make_valid(row2);

    // apply sudoku rows to the grid array
    apply_to(grid, { row0, row1, row2 });
    print(grid);

    if (check_valid(grid)) {
      std::cout << "grid valid\n";
    } else {
      std::cout << "grid invalid\n";
      break;
    }
  }
  return 0;
}
