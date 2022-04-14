// Copyright (c) 2022 Cameron King.
// Dual licensed under MIT and GPLv2 with OpenSSL exception.
// See LICENSE for details.

#ifndef RACINGWEB_SRC_RACEUTIL_H_
#define RACINGWEB_SRC_RACEUTIL_H_

#include <vector>

#include "src/Car.h"

/**
 * checks if any cars in the two vectors provided are the same object
 * @param a left operand to compare
 * @param b right operand to compare
 * @return true if any cars in either vector are the same object
 */
bool DoAnyCarsMatch(std::vector<const Car *> const &a,
                    std::vector<const Car *> const &b);

#endif  // RACINGWEB_SRC_RACEUTIL_H_
