// Copyright (c) 2022 Cameron King.
// Dual licensed under MIT and GPLv2 with OpenSSL exception.
// See LICENSE for details.

#ifndef RACINGWEB_SRC_RESULT_H_
#define RACINGWEB_SRC_RESULT_H_

#include "src/Car.h"

/// @brief a single finish line result
struct Result {
  /**
   * @brief create a single finish line result
   * @param car the car that finished
   * @param place what place that car came in
   */
  Result(Car *car, int place) : car(car), place(place) {}
  /// @brief a pointer to the car in question
  Car *car;
  /// @brief what place did the car come in for this heat
  int place;
};

#endif  // RACINGWEB_SRC_RESULT_H_
