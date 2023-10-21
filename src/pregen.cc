// Copyright (c) 2023 Cameron King.
// Dual licensed under MIT and GPLv2 with OpenSSL exception.
// See LICENSE for details.
/// @file

#include "src/pregen.h"

std::vector<std::vector<const Car *>> LoadPreGeneratedSchedule(
    const std::vector<Car> &roster) {
  auto cars = roster.size();
  auto schedule{std::vector<std::vector<const Car *>>()};

  // this is a FOUR LANE pregen.
  // less than four racers should use fewer lanes
  if (cars < 4) {
    return schedule;
  }

  // record the heat configuration from https://stanpope.net/ppngen.html
  int first_heat[4] = {0};
  switch (cars) {
    case 4:
      first_heat[0] = 1;
      first_heat[1] = 4;
      first_heat[2] = 3;
      first_heat[3] = 2;
      break;
    case 5:
    case 6:
    case 7:
      first_heat[0] = 1;
      first_heat[1] = 3;
      first_heat[2] = 5;
      first_heat[3] = 2;
      break;
    case 8:
      first_heat[0] = 1;
      first_heat[1] = 3;
      first_heat[2] = 5;
      first_heat[3] = 8;
      break;
    case 9:
      first_heat[0] = 1;
      first_heat[1] = 3;
      first_heat[2] = 5;
      first_heat[3] = 9;
      break;
    case 10:
      first_heat[0] = 1;
      first_heat[1] = 3;
      first_heat[2] = 5;
      first_heat[3] = 10;
      break;
    case 11:
      first_heat[0] = 1;
      first_heat[1] = 3;
      first_heat[2] = 5;
      first_heat[3] = 11;
      break;
    case 12:
      first_heat[0] = 1;
      first_heat[1] = 3;
      first_heat[2] = 7;
      first_heat[3] = 12;
      break;
    default:
      first_heat[0] = 1;
      first_heat[1] = 3;
      first_heat[2] = 7;
      first_heat[3] = 6;
      break;
  }

  for (int i = 0; i < cars; i++) {
    schedule.emplace_back(BuildPreGenHeat(
        roster, (first_heat[0] - 1 + i) % cars, (first_heat[1] - 1 + i) % cars,
        (first_heat[2] - 1 + i) % cars, (first_heat[3] - 1 + i) % cars));
  }

  return schedule;
}

std::vector<const Car *> BuildPreGenHeat(const std::vector<Car> &roster,
                                         const int one, const int two,
                                         const int three, const int four) {
  auto heat{std::vector<const Car *>()};
  heat.emplace_back(&roster[one]);
  heat.emplace_back(&roster[two]);
  heat.emplace_back(&roster[three]);
  heat.emplace_back(&roster[four]);
  return heat;
}
