// Copyright (c) 2022 Cameron King.
// Dual licensed under MIT and GPLv2 with OpenSSL exception.
// See LICENSE for details.
/// @file

#include "src/RacingWebApplication.h"

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
  lineup_container =
      vert_layout->addWidget(std::make_unique<Wt::WContainerWidget>());

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

  // lay out the lineup in a grid
  standings_container =
      vert_layout->addWidget(std::make_unique<Wt::WContainerWidget>());

  return container;
}
