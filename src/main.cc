// Copyright (c) 2022 Cameron King.
// Dual licensed under MIT and GPLv2 with OpenSSL exception.
// See LICENSE for details.
/// @file

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WGridLayout.h>
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
   * @brief generates the schedule_text and outputs schedule_text information
   */
  void GenerateSchedule();
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

  // start with the setup tab visible
  setup_tab->select();
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
  for (int i = 0; i < cars; i++) {
    roster.emplace_back(i + 1);
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

  for (int i = 0; i < cars; i++) {
    schedule_summary << "Heat " << i + 1 << ": ";
    for (int lane = 0; lane < lanes; lane++) {
      schedule_summary << schedule[i][lane]->number << " ";
    }
    schedule_summary << "<br />";
  }

  schedule_text->setText(schedule_summary.str());
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

  container->addWidget(std::make_unique<Wt::WText>("Run container"));

  return container;
}

std::unique_ptr<Wt::WContainerWidget>
RacingWebApplication::BuildStandingsContainer() {
  auto container = std::make_unique<Wt::WContainerWidget>();
  container->setPadding(Wt::WLength(10), Wt::AllSides);

  container->addWidget(std::make_unique<Wt::WText>("Standings container"));

  return container;
}

int main(int argc, char **argv) {
  return Wt::WRun(argc, argv, [](const Wt::WEnvironment &env) {
    return std::make_unique<RacingWebApplication>(env);
  });
}
