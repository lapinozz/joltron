
#include <Arduino.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

#include "SimpleI2C.h"
#include "str.hpp"
#include "utils.hpp"
#include "Display.hpp"
#include "font.hpp"
#include "DisplayBuffer.hpp"
#include "input.hpp"
#include "LedController.hpp"
#include "menu.hpp"
#include "Games.hpp"
#include "glyphs.hpp"

SoftwareSerial dfPlayerSerial(3, 4); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

LedController ledController;

SimpleI2C SI2C;
Display display(SI2C);

Input input;

MenuDisplay menu;

GameRunner gameRunner;

void setup()
{
  Random::init();

  dfPlayerSerial.begin(9600);

  if (!myDFPlayer.begin(dfPlayerSerial))
  {
    while(true);
  }
  myDFPlayer.volume(1); //Set volume value. From 0 to 30
  //myDFPlayer.outputSetting(false, 31);  //Set volume value. From 0 to 30

  //myDFPlayer.play(1);

  SI2C.init(true);
  
  display.selectScreen(Display::Screen::All);

  display.init();
  display.clearRect();

  input.init();

  ledController.init();

  menu.setMenu(Menus::Main);

  gameRunner.setGame(0);
}

void loop()
{
  const auto loopStartTime = millis();

  input.update();

  const auto menuAction = menu.update(display, input, ledController);

  if(menuAction == MenuAction::Play)
  {
    gameRunner.setGame(0);
  }

  gameRunner.update(display, input, ledController);

  ledController.display();

  //if (myDFPlayer.available()) {
  //  printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  //}

  const auto loopEndTime = millis();
  display.selectMenu();
  //display.clearRect(Display::Width - (Font::charWidth + 1) * 3, Display::Height - Font::charHeight, (Font::charWidth + 1) * 3, 8);
  display.clearRect(Display::Width - (Font::charWidth + 1) * 3, 0, (Font::charWidth + 1) * 3, 8);
  //display.print(loopEndTime - loopStartTime);
  display.print((unsigned long)gameRunner.state.deltaTime);
}