#include "App.hpp"

App app;

void setup()
{
  app.init();
  app.setPhase(App::Phase::Joining);
}

void loop()
{
  app.update();
}