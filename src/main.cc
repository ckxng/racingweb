// Copyright (c) 2022 Cameron King.
// Dual licensed under MIT and GPLv2 with OpenSSL exception.
// See LICENSE for details.
//// @file

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include <memory>

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
  /// @brief text box for number of cars to race
  Wt::WLineEdit *number_of_cars;
  //// @brief text box for number of lanes on the track
  Wt::WLineEdit *number_of_lanes;
  /// @brief output text containing schedule summary
  Wt::WText *schedule;

  /**
   * @brief generates the schedule and outputs schedule information
   */
  void GenerateSchedule();
};

RacingWebApplication::RacingWebApplication(const Wt::WEnvironment &env)
    : WApplication(env) {
  setTitle("Racing Web");

  root()->addWidget(std::make_unique<Wt::WText>("How many cars?"));

  number_of_cars = root()->addWidget(std::make_unique<Wt::WLineEdit>("12"));
  number_of_cars->setFocus();

  root()->addWidget(std::make_unique<Wt::WBreak>());

  root()->addWidget(std::make_unique<Wt::WText>("How many lanes?"));

  number_of_lanes = root()->addWidget(std::make_unique<Wt::WLineEdit>("4"));

  auto button =
      root()->addWidget(std::make_unique<Wt::WPushButton>("Generate schedule"));
  button->setMargin(5, Wt::Side::Left);  // add 5 pixels margin
  button->clicked().connect(this, &RacingWebApplication::GenerateSchedule);

  root()->addWidget(std::make_unique<Wt::WBreak>());

  schedule = root()->addWidget(std::make_unique<Wt::WText>());

  root()->addWidget(std::make_unique<Wt::WBreak>());
}

void RacingWebApplication::GenerateSchedule() {
  schedule->setText(number_of_cars->text() + " cars x " +
                    number_of_lanes->text() + " lanes");
}

int main(int argc, char **argv) {
  return Wt::WRun(argc, argv, [](const Wt::WEnvironment &env) {
    return std::make_unique<RacingWebApplication>(env);
  });
}
