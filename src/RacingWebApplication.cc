// Copyright (c) 2022 Cameron King.
// Dual licensed under MIT and GPLv2 with OpenSSL exception.
// See LICENSE for details.
/// @file

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

  auto schedule_summary{std::stringstream()};

  // all cars must be created before generating the race schedule
  // take this opportunity to reset the results as well
  roster = std::vector<Car>();
  results = std::vector<std::vector<std::unique_ptr<Result>>>();
  for (int i = 0; i < cars; i++) {
    roster.emplace_back(i + 1);
    results.emplace_back(std::vector<std::unique_ptr<Result>>());
  }

  auto initial_schedule{std::vector<std::vector<const Car *>>()};
  if (lanes == 4 && cars <= 13) {
    // use pre-generated races for 4 lane tracks up to 13 racers
    initial_schedule = LoadPreGeneratedSchedule(roster);
  } else {
    // generate the race schedule
    for (int i = 0; i < cars; i++) {
      auto heat{std::vector<const Car *>()};
      for (int lane = 0; lane < lanes; lane++) {
        heat.emplace_back(&roster[(i + lane) % cars]);
      }
      initial_schedule.emplace_back(heat);
    }
  }

  // try to arrange the schedule so that cars are not in adjacent heats
  schedule = std::vector<std::vector<const Car *>>();

  // move the first heat into the optimized_schedule
  schedule.emplace_back(initial_schedule[0]);
  initial_schedule.erase(initial_schedule.begin());

  // populate the optimized_schedule, preferring arrangements where the
  // cars are not in adjacent heats
  for (int i = 1; i < cars; i++) {
    int next_heat{0};

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

  // move to run_tab
  run_tab->enable();
  standings_tab->enable();
  run_tab->select();

  // once generated, page should be reloaded to change the number of lanes
  // to start from a clean slate
  number_of_lanes->disable();
}

void RacingWebApplication::SetCurrentHeat(int heat) {
  // if the heat value is negative or exceeds size, then we must be done
  // racing.  update the UI to indicate that.
  if (heat < 0 || heat > schedule.size()) {
    FinishRacing();
    return;
  }
  current_heat = heat;

  // set title for run tab
  run_title->setText("Heat " + std::to_string(current_heat + 1) + " of " +
                     std::to_string(schedule.size()));

  // update layouts on run tab
  UpdateLineupContainer();

  // update preview of next heat
  auto on_deck{IdentifyHeatOnDeck()};
  if (on_deck >= 0) {
    auto preview_builder = std::stringstream();
    preview_builder << "On Deck - Heat " << std::to_string(on_deck + 1) << ": ";
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
  auto found_first{false};
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

void RacingWebApplication::UpdateLineupContainer() {
  auto lanes = static_cast<int>(schedule[current_heat].size() & INT_MAX);

  lineup_container->clear();

  // lay out the lineup in a grid
  auto lineup_grid_layout =
      lineup_container->setLayout(std::make_unique<Wt::WGridLayout>());

  // set the last column to take up all excess space
  lineup_grid_layout->setColumnStretch(0, 0);  // lane
  lineup_grid_layout->setColumnStretch(1, 0);  // car number
  lineup_grid_layout->setColumnStretch(2, 0);  // car name
  lineup_grid_layout->setColumnStretch(3, 0);  // driver name
  for (int i = 0; i < schedule[current_heat].size(); i++) {
    lineup_grid_layout->setColumnStretch(i + 4, 0);  // places
  }
  lineup_grid_layout->setColumnStretch(lanes + 4, 100);

  // read the schedule data and fill in the grid layout
  place_button_matrix = std::vector<std::vector<Wt::WPushButton *>>();
  auto show_car_name{false}, show_driver_name{false};
  for (int i = 0; i < schedule[current_heat].size(); i++) {
    lineup_grid_layout->addWidget(
        std::make_unique<Wt::WText>(std::to_string(i + 1)), i + 1, 0);
    lineup_grid_layout->addWidget(
        std::make_unique<Wt::WText>(schedule[current_heat][i]->number), i + 1,
        1);

    if (!schedule[current_heat][i]->car.empty()) {
      show_car_name = true;
      lineup_grid_layout->addWidget(
          std::make_unique<Wt::WText>(schedule[current_heat][i]->car), i + 1,
          2);
    }

    if (!schedule[current_heat][i]->car.empty()) {
      show_driver_name = true;
      lineup_grid_layout->addWidget(
          std::make_unique<Wt::WText>(schedule[current_heat][i]->driver), i + 1,
          3);
    }

    // buttons to indicate places
    place_button_matrix.emplace_back(std::vector<Wt::WPushButton *>());
    for (int place = 0; place < schedule[current_heat].size(); place++) {
      place_button_matrix[i].emplace_back(lineup_grid_layout->addWidget(
          std::make_unique<Wt::WPushButton>(std::to_string(place + 1)), i + 1,
          place + 4));

      place_button_matrix[i][place]->clicked().connect([this, i, place]() {
        MarkPlace(*schedule[current_heat][i], i, place);
      });
    }
  }

  accept_results_button = lineup_grid_layout->addWidget(
      std::make_unique<Wt::WPushButton>("Accept Results"), lanes + 1, 4, 1,
      lanes);
  accept_results_button->disable();
  accept_results_button->clicked().connect(
      [this]() { SetCurrentHeat(IdentifyNextHeat()); });

  auto reset_results_button = lineup_grid_layout->addWidget(
      std::make_unique<Wt::WPushButton>("Clear Results"), lanes + 2, 4, 1,
      lanes);
  reset_results_button->clicked().connect([this]() {
    results[current_heat] = std::vector<std::unique_ptr<Result>>();
    UpdateLineupContainer();
  });

  // first row - do this last so we can hide unused columns
  lineup_grid_layout->addWidget(std::make_unique<Wt::WText>("Lane"), 0, 0);
  lineup_grid_layout->addWidget(std::make_unique<Wt::WText>("Car"), 0, 1);
  if (show_car_name) {
    lineup_grid_layout->addWidget(std::make_unique<Wt::WText>("Name"), 0, 2);
  }
  if (show_driver_name) {
    lineup_grid_layout->addWidget(std::make_unique<Wt::WText>("Driver"), 0, 3);
  }
  lineup_grid_layout->addWidget(std::make_unique<Wt::WText>("Place"), 0, 4, 1,
                                lanes);

  // add blank text so last column will stretch
  lineup_grid_layout->addWidget(std::make_unique<Wt::WText>(), 0, 4 + lanes);
}

void RacingWebApplication::MarkPlace(const Car &car, const int lane,
                                     const int place) {
  // if this is the first record in this heat, create the array
  if (results[current_heat].empty()) {
    results[current_heat] =
        std::vector<std::unique_ptr<Result>>(schedule[current_heat].size());
  }

  // place the heat
  results[current_heat][lane] = std::make_unique<Result>(car, place);

  // disable no longer relevant buttons
  for (int i = 0; i < schedule[current_heat].size(); i++) {
    place_button_matrix[lane][i]->disable();
    place_button_matrix[lane][i]->setText("x");
    place_button_matrix[i][place]->disable();
    place_button_matrix[i][place]->setText("x");
  }
  place_button_matrix[lane][place]->setText("O");

  // check if all results are now in
  auto any_results_open = false;
  for (const auto &item : results[current_heat]) {
    // cppcheck-suppress useStlAlgorithm
    // only suppress for now, refactor this to use algo AND be readable
    if (!item) {
      any_results_open = true;
      break;
    }
  }
  if (!any_results_open) {
    accept_results_button->enable();
  }
}
void RacingWebApplication::FinishRacing() {
  run_title->setText("Finished");
  lineup_container->clear();
  lineup_container->addWidget(std::make_unique<Wt::WText>("Done racing!"));

  UpdateStandingsContainer();
  standings_tab->select();
}

void RacingWebApplication::UpdateStandingsContainer() {
  standings_container->clear();

  // lay out the standings in a grid
  auto standings_grid_layout =
      standings_container->setLayout(std::make_unique<Wt::WGridLayout>());

  // set the last column to take up all excess space
  standings_grid_layout->setColumnStretch(0, 0);  // place
  standings_grid_layout->setColumnStretch(1, 0);  // car number
  standings_grid_layout->setColumnStretch(2, 0);  // car name
  standings_grid_layout->setColumnStretch(3, 0);  // driver name
  standings_grid_layout->setColumnStretch(4, 100);

  auto final_standings = CalculateFinalStandings();

  // read the schedule data and fill in the grid layout
  auto show_car_name{false}, show_driver_name{false};
  for (int i = 0; i < final_standings.size(); i++) {
    standings_grid_layout->addWidget(
        std::make_unique<Wt::WText>(std::to_string(i + 1)), i + 1, 0);
    standings_grid_layout->addWidget(
        std::make_unique<Wt::WText>(final_standings[i]->number), i + 1, 1);
    if (!final_standings[i]->car.empty()) {
      show_car_name = true;
      standings_grid_layout->addWidget(
          std::make_unique<Wt::WText>(final_standings[i]->car), i + 1, 2);
    }
    if (!final_standings[i]->driver.empty()) {
      show_driver_name = true;
      standings_grid_layout->addWidget(
          std::make_unique<Wt::WText>(final_standings[i]->driver), i + 1, 3);
    }
  }

  // first row - do this last so we can hide unused columns
  standings_grid_layout->addWidget(std::make_unique<Wt::WText>("Place"), 0, 0);
  standings_grid_layout->addWidget(std::make_unique<Wt::WText>("Car"), 0, 1);
  if (show_car_name) {
    standings_grid_layout->addWidget(std::make_unique<Wt::WText>("Name"), 0, 2);
  }
  if (show_driver_name) {
    standings_grid_layout->addWidget(std::make_unique<Wt::WText>("Driver"), 0,
                                     3);
  }
  // add blank text so last column will stretch
  standings_grid_layout->addWidget(std::make_unique<Wt::WText>(), 0, 4);
}
std::vector<const Car *> RacingWebApplication::CalculateFinalStandings() {
  auto final_standings = std::vector<const Car *>();
  for (const auto &item : roster) {
    // cppcheck-suppress useStlAlgorithm,unmatchedSuppression
    // not transforming
    final_standings.emplace_back(&item);
  }

  // create map to store score calculations in
  auto scores = std::map<const Car *, int>();
  std::for_each(roster.begin(), roster.end(),
                [&scores](const auto &x) { scores[&x] = 0; });

  // add up the results
  for (const auto &heat : results) {
    std::for_each(heat.begin(), heat.end(),
                  [&scores](const auto &x) { scores[x->car] += x->place; });
  }

  // final standings sort
  std::sort(final_standings.begin(), final_standings.end(),
            [&scores](const auto &a, const auto &b) {
              return scores[a] < scores[b];
            });

  return std::move(final_standings);
}
