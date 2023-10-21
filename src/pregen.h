// Copyright (c) 2023 Cameron King.
// Dual licensed under MIT and GPLv2 with OpenSSL exception.
// See LICENSE for details.
/// @file

#ifndef RACINGWEB_SRC_PREGEN_H_
#define RACINGWEB_SRC_PREGEN_H_

#include <vector>

#include "src/Car.h"

/**
 * spits out a pre-generated schedule for 4 lanes using a more optimal pattern
 * @param roster vector of cars in the race roster
 * @return completed race schedule as a vector of heats, each heat consisting of
 * a vector of 4 cars
 */
std::vector<std::vector<const Car *>> LoadPreGeneratedSchedule(
    const std::vector<Car> &roster);

/**
 * helper function to generate a single heat.
 * used in conjuction with https://stanpope.net/ppngen.html
 * params one, two, three, four match indexes in the roster vector
 * @param one the roster vector index of the car to place in lane one
 * @param two the roster vector index of the car to place in lane two
 * @param three the roster vector index of the car to place in lane three
 * @param four the roster vector index of the car to place in lane four
 * @return a heat which can be added to the schedule
 */
std::vector<const Car *> BuildPreGenHeat(const std::vector<Car> &roster,
                                         const int one, const int two,
                                         const int three, const int four);

#endif  // RACINGWEB_SRC_PREGEN_H_
