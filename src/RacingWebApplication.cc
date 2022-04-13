// Copyright (c) 2022 Cameron King.
// Dual licensed under MIT and GPLv2 with OpenSSL exception.
// See LICENSE for details.

#include "src/RacingWebApplication.h"

RacingWebApplication::RacingWebApplication(const Wt::WEnvironment &env)
    : WApplication(env) {
  setTitle("Racing Web");

  // the main window frame is a tabbed interface
  tabs = root()->addNew<Wt::WTabWidget>();

  // build the content for each tab
  setup_tab = tabs->addTab(BuildSetupContainer(), "Setup");
  run_tab = tabs->addTab(BuildRunContainer(), "Run");
  standings_tab = tabs->addTab(BuildStandingsContainer(), "Standings");

  // start with the setup tab visible, and others disabled
  setup_tab->select();
  run_tab->disable();
  standings_tab->disable();
}

void RacingWebApplication::GenerateSchedule() {
  int cars, lanes;

  // failure to parse is likely the result of an accidental button click
  // just ignore it
  try {
    cars = std::stoi(number_of_cars->text());
    lanes = std::stoi(number_of_lanes->text());
  } catch (std::invalid_argument const &invalid_argument) {
    return;
  } catch (std::out_of_range const &out_of_range) {
    return;
  }

  // non-positive values are treated the same way
  if (cars < 1 || lanes < 1) {
    return;
  }

  // cap the number of lanes at the number of cars
  if (lanes > cars) {
    lanes = cars;
  }

  auto schedule_summary = std::stringstream();

  // calculate lane offsets to mix cars
  auto lane_offsets = std::vector<int>();
  for (int i = 0, offset = 0; i < lanes; i++, offset += i) {
    lane_offsets.emplace_back(offset);
  }

  // all cars must be created before generating the race schedule
  // take this opportunity to reset the results as well
  roster = {};
  results = {};
  for (int i = 0; i < cars; i++) {
    roster.emplace_back(i + 1);
    results.emplace_back(std::vector<Result>());
  }

  // generate the race schedule
  auto initial_schedule = std::vector<std::vector<Car *>>();
  for (int i = 0; i < cars; i++) {
    auto heat = std::vector<Car *>();
    for (int lane = 0; lane < lanes; lane++) {
      int const roster_pos = (i + lane_offsets[lane]) % cars;
      heat.emplace_back(&roster[roster_pos]);
    }
    initial_schedule.emplace_back(heat);
  }

  // try to arrange the schedule so that cars are not in adjacent heats
  schedule = std::vector<std::vector<Car *>>();

  // move the first heat into the optimized_schedule
  schedule.emplace_back(initial_schedule[0]);
  initial_schedule.erase(initial_schedule.begin());

  // populate the optimized_schedule, preferring arrangements where the
  // cars are not in adjacent heats
  for (int i = 1; i < cars; i++) {
    int next_heat = 0;

    // look for a better next heat, if possible by comparing with the
    // previous heat in the optimized schedule
    for (int proposed_idx = 0; proposed_idx < initial_schedule.size();
         proposed_idx++) {
      if (!DoAnyCarsMatch(schedule[i - 1], initial_schedule[proposed_idx])) {
        next_heat = proposed_idx;
        break;
      }
    }

    // move the next heat into the optimized schedule
    schedule.emplace_back(initial_schedule[next_heat]);
    initial_schedule.erase(initial_schedule.begin() + next_heat);
  }

  // display the standings
  for (int i = 0; i < cars; i++) {
    schedule_summary << "Heat " << i + 1 << ": ";
    for (int lane = 0; lane < lanes; lane++) {
      schedule_summary << schedule[i][lane]->number << " ";
    }
    schedule_summary << "<br />";
  }
  schedule_text->setText(schedule_summary.str());

  // update the current heat and enable run and standings tabs
  SetCurrentHeat(0);
  run_tab->enable();
  standings_tab->enable();
}

void RacingWebApplication::SetCurrentHeat(int heat) {
  if (heat < 0 || heat > schedule.size()) {
    return;
  }
  current_heat = heat;

  // set title for run tab
  run_title->setText("Heat " + std::to_string(current_heat + 1) + " of " +
                     std::to_string(schedule.size()));

  // build heat lineup for run tab
  // lineup_grid_layout

  // build place indicator for run tab
  // place_grid_layout

  // update preview of next heat
  auto on_deck = IdentifyHeatOnDeck();
  if (on_deck >= 0) {
    auto preview_builder = std::stringstream();
    preview_builder << "Next - Heat " << std::to_string(on_deck + 1) << ": ";
    for (int i = 0; i < schedule[on_deck].size(); i++) {
      if (i != 0) {
        preview_builder << ", ";
      }
      preview_builder << schedule[on_deck][i]->number;
    }
    heat_preview_text->setText(preview_builder.str());
  } else {
    heat_preview_text->setText("No more heats to run");
  }
}

int RacingWebApplication::IdentifyNextHeat() const {
  for (int i = 0; i < results.size(); i++) {
    if (results[i].empty()) {
      return i;
    }
  }
  return -1;
}

int RacingWebApplication::IdentifyHeatOnDeck() const {
  auto found_first = false;
  for (int i = 0; i < results.size(); i++) {
    if (results[i].empty()) {
      if (found_first) {
        return i;
      } else {
        found_first = true;
      }
    }
  }
  return -1;
}
