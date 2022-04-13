// Copyright (c) 2022 Cameron King.
// Dual licensed under MIT and GPLv2 with OpenSSL exception.
// See LICENSE for details.
/// @file

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WGridLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WLineEdit.h>
#include <Wt/WMenuItem.h>
#include <Wt/WPanel.h>
#include <Wt/WPushButton.h>
#include <Wt/WTabWidget.h>
#include <Wt/WText.h>
#include <Wt/WVBoxLayout.h>

#include <memory>
#include <sstream>
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
  explicit Car(int number, std::string car = "", std::string driver = "")
      : car(std::move(car)), driver(std::move(driver)) {
    this->number = std::to_string(number);
  }

  /**
   * @brief create a racer
   * @param number car number, alphanumeric, only required argument
   * @param car name of the car (defaults to "")
   * @param driver name of the driver (defaults to "")
   */
  explicit Car(std::string number, std::string car = "",
               std::string driver = "")
      : number(std::move(number)),
        car(std::move(car)),
        driver(std::move(driver)) {}

  /// @brief alphanumeric car number
  std::string number;
  /// @brief car name ("" represents unspecified)
  std::string car = "";
  /// @brief driver name ("" represents unspecified)
  std::string driver = "";
};

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

/**
 * @brief application state container class
 */
class RacingWebApplication : public Wt::WApplication {
 public:
  /**
   * @brief initializes the application
   * @param env application environment
   */
  explicit RacingWebApplication(const Wt::WEnvironment &env);

 private:
  /**
   * @brief builds the setup container and saves key elements as members
   * @return unique pointer to setup container
   */
  std::unique_ptr<Wt::WContainerWidget> BuildSetupContainer();

  /**
   * @brief builds the run container and saves key elements as members
   * @return unique pointer to run container
   */
  std::unique_ptr<Wt::WContainerWidget> BuildRunContainer();

  /**
   * @brief builds the standings container and saves key elements as members
   * @return unique pointer to standings container
   */
  std::unique_ptr<Wt::WContainerWidget> BuildStandingsContainer();

  /**
   * @brief generates the schedule
   *
   * This method generates schedules where number_of_cars and number_of_lanes
   * can have their text contents cast to and integer and
   * 0 < number_of_cars and 0 < number_of_lanes <= number_of_cars.
   * number_of_lanes will be capped to number_of_cars.  The roster and results
   * data members are populated, and a schedule is created.
   */
  void GenerateSchedule();

  /**
   * @brief sets current_heat and updates related text
   *
   * Updates the current_heat member and text elements on the run_tab related
   * to the current heat.  Takes no action if given an invalid heat.
   *
   * @param heat the current heat where 0 <= heat < schedule.size()
   */
  void SetCurrentHeat(int heat);

  /**
   * @brief determine which heat will run next
   * @return the next index in results with size() == 0 or -1 if none
   */
  [[nodiscard]] int IdentifyNextHeat() const;

  /**
   * @brief determine which heat will run next
   * @return the second index in results with size() == 0 or -1 if none
   */
  [[nodiscard]] int IdentifyHeatOnDeck() const;

  /// @brief text box for number of cars to race
  Wt::WLineEdit *number_of_cars;

  /// @brief text box for number of lanes on the track
  Wt::WLineEdit *number_of_lanes;

  /// @brief output text containing schedule_text summary
  Wt::WText *schedule_text;

  /// @brief collection of main tabs
  Wt::WTabWidget *tabs;

  /// @brief setup tab
  Wt::WMenuItem *setup_tab;

  /// @brief run tab
  Wt::WMenuItem *run_tab;

  /// @brief standings tab
  Wt::WMenuItem *standings_tab;

  /// @brief the cars that will be raced
  std::vector<Car> roster;

  /// @brief the race schedule, with pointers to cars on the roster
  std::vector<std::vector<Car *>> schedule;

  /**
   * @brief the finish line results
   *
   * The correct number of heats are pre-created when the schedule is
   * generated.  A heat with a result length of zero indicatees a heat which
   * has not been run yet.
   */
  std::vector<std::vector<Result>> results;

  /// @brief what heat are we currently on (0-indexed, to match schedule)
  int current_heat = 0;

  /// @brief the title of the run container
  Wt::WText *run_title;

  /// @brief the grid layout for the current heat lineup
  Wt::WGridLayout *lineup_grid_layout;

  /// @brief the grid layout for placing cars to indicate results
  Wt::WGridLayout *place_grid_layout;

  /// @brief the output text previewing the lineup for the next heat
  Wt::WText *heat_preview_text;
};

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

std::unique_ptr<Wt::WContainerWidget>
RacingWebApplication::BuildSetupContainer() {
  auto container = std::make_unique<Wt::WContainerWidget>();

  // basic vertical layout
  auto vert_layout = container->setLayout(std::make_unique<Wt::WVBoxLayout>());
  vert_layout->addWidget(std::make_unique<Wt::WText>("Race Setup"))
      ->setHtmlTagName("h1");

  // lay out the form in a grid
  auto form_grid_layout =
      vert_layout->addLayout(std::make_unique<Wt::WGridLayout>());

  // set the third column to take up all excess space
  form_grid_layout->setColumnStretch(0, 0);
  form_grid_layout->setColumnStretch(1, 0);
  form_grid_layout->setColumnStretch(2, 100);

  form_grid_layout->addWidget(std::make_unique<Wt::WText>("How many cars?"), 0,
                              0);
  number_of_cars =
      form_grid_layout->addWidget(std::make_unique<Wt::WLineEdit>("12"), 0, 1);
  number_of_cars->setFocus();

  form_grid_layout->addWidget(std::make_unique<Wt::WText>("How many lanes?"), 1,
                              0);

  number_of_lanes =
      form_grid_layout->addWidget(std::make_unique<Wt::WLineEdit>("4"), 1, 1);

  auto button = form_grid_layout->addWidget(
      std::make_unique<Wt::WPushButton>("Generate schedule"), 2, 1);
  button->clicked().connect(this, &RacingWebApplication::GenerateSchedule);

  // empty widget at the end to let the third column stretch out
  form_grid_layout->addWidget(std::make_unique<Wt::WText>(""), 2, 2);

  schedule_text = vert_layout->addWidget(std::make_unique<Wt::WText>());

  return container;
}

std::unique_ptr<Wt::WContainerWidget>
RacingWebApplication::BuildRunContainer() {
  auto container = std::make_unique<Wt::WContainerWidget>();
  container->setPadding(Wt::WLength(10), Wt::AllSides);

  // basic vertical layout
  auto vert_layout = container->setLayout(std::make_unique<Wt::WVBoxLayout>());
  run_title = vert_layout->addWidget(std::make_unique<Wt::WText>());
  run_title->setHtmlTagName("h1");

  // lay out the lineup in a grid
  lineup_grid_layout =
      vert_layout->addLayout(std::make_unique<Wt::WGridLayout>());

  // lay out the place indicator in a grid
  place_grid_layout =
      vert_layout->addLayout(std::make_unique<Wt::WGridLayout>());

  // add sneak peek of the next heat lineup
  heat_preview_text = vert_layout->addWidget(std::make_unique<Wt::WText>(""));

  return container;
}

std::unique_ptr<Wt::WContainerWidget>
RacingWebApplication::BuildStandingsContainer() {
  auto container = std::make_unique<Wt::WContainerWidget>();
  container->setPadding(Wt::WLength(10), Wt::AllSides);

  // basic vertical layout
  auto vert_layout = container->setLayout(std::make_unique<Wt::WVBoxLayout>());
  vert_layout->addWidget(std::make_unique<Wt::WText>("Standings"))
      ->setHtmlTagName("h1");

  return container;
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

int main(int argc, char **argv) {
  return Wt::WRun(argc, argv, [](const Wt::WEnvironment &env) {
    return std::make_unique<RacingWebApplication>(env);
  });
}
