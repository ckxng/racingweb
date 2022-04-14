// Copyright (c) 2022 Cameron King.
// Dual licensed under MIT and GPLv2 with OpenSSL exception.
// See LICENSE for details.

#ifndef RACINGWEB_SRC_RACINGWEBAPPLICATION_H_
#define RACINGWEB_SRC_RACINGWEBAPPLICATION_H_

#include <Wt/WApplication.h>
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

#include <algorithm>
#include <climits>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "src/Car.h"
#include "src/Result.h"
#include "src/raceutil.h"

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
   * @brief read the current_heat and update the lineup for the race tab
   */
  void UpdateLineupContainer();

  /**
   * @brief read results and return an ordered vector of winners
   *
   * the car in index 0 came in first place, index 1 is second place, and so on
   * @return the ordered list of winners
   */
  std::vector<const Car *> CalculateFinalStandings();

  /**
   * @brief read the results and update the standings tab
   */
  void UpdateStandingsContainer();

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
   * @brief update the ui to indicate the race is over
   *
   * resets the contents of the run tab to show the race is finished and
   * progress to the standings tab
   */
  void FinishRacing();

  /**
   * marks the place of a car in the current heat
   * @param lane which lane the car is in
   * @param place the place the car came in
   */
  void MarkPlace(const Car &car, int lane, int place);

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
  std::vector<std::vector<const Car *>> schedule;

  /**
   * @brief the finish line results
   *
   * The correct number of heats are pre-created when the schedule is
   * generated.  A heat with a result length of zero indicatees a heat which
   * has not been run yet.
   */
  std::vector<std::vector<std::unique_ptr<Result>>> results;

  /// @brief what heat are we currently on (0-indexed, to match schedule)
  int current_heat = 0;

  /// @brief the title of the run container
  Wt::WText *run_title;

  /// @brief the grid container for the current heat lineup
  Wt::WContainerWidget *lineup_container;

  /// @brief the grid container for the current heat lineup
  Wt::WContainerWidget *standings_container;

  /// @brief the output text previewing the lineup for the next heat
  Wt::WText *heat_preview_text;

  /// @brief matrix of buttons that indicate finish line places
  std::vector<std::vector<Wt::WPushButton *>> place_button_matrix;

  /// @brief accept final results
  Wt::WPushButton *accept_results_button;
};

#endif  // RACINGWEB_SRC_RACINGWEBAPPLICATION_H_
