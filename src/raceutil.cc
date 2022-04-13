// Copyright (c) 2022 Cameron King.
// Dual licensed under MIT and GPLv2 with OpenSSL exception.
// See LICENSE for details.

#include "src/raceutil.h"

bool DoAnyCarsMatch(std::vector<Car *> const &a, std::vector<Car *> const &b) {
  for (const auto &item_a : a) {
    for (const auto &item_b : b) {
      // cppcheck-suppress useStlAlgorithm
      // these loops are trivial
      if (item_a == item_b) {
        return true;
      }
    }
  }
  return false;
}
