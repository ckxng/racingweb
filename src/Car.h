// Copyright (c) 2022 Cameron King.
// Dual licensed under MIT and GPLv2 with OpenSSL exception.
// See LICENSE for details.
/// @file

#ifndef RACINGWEB_SRC_CAR_H_
#define RACINGWEB_SRC_CAR_H_

#include <string>
#include <utility>

/// @brief a race participant
struct Car {
  /**
   * @brief create a racer
   * @param number car number, alphanumeric, only required argument
   * @param car name of the car (defaults to "")
   * @param driver name of the driver (defaults to "")
   */
  explicit Car(const int number, const std::string &car = "",
               const std::string &driver = "") {
    this->number = std::to_string(number);
    this->car = car;
    this->driver = driver;
  }

  /**
   * @brief create a racer
   * @param number car number, alphanumeric, only required argument
   * @param car name of the car (defaults to "")
   * @param driver name of the driver (defaults to "")
   */
  explicit Car(const std::string &number, const std::string &car = "",
               const std::string &driver = "") {
    this->number = number;
    this->car = car;
    this->driver = driver;
  }

  /// @brief alphanumeric car number
  std::string number;
  /// @brief car name ("" represents unspecified)
  std::string car = "";
  /// @brief driver name ("" represents unspecified)
  std::string driver = "";
};

#endif  // RACINGWEB_SRC_CAR_H_
