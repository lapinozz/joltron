
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

class App
{
public:
  enum class Phase : uint8_t
  {
    Joining,
    Menu,
    Paused,
    Game,

    Count
  };

private:
  SoftwareSerial dfPlayerSerial = {3, 4}; // RX, TX
  DFRobotDFPlayerMini myDFPlayer;
  LedController ledController;
  SimpleI2C SI2C;
  Display display = {SI2C};
  Input input;
  MenuDisplay menu;
  GameRunner gameRunner;

  Phase phase;
  
  uint32_t lastFrame = {};
  uint8_t deltaTime = {};

  using PhaseFunction = void (App::*)(void);
  struct PhaseFunctions
  {
    PhaseFunction onStart{};
    PhaseFunction onIdle{};
    PhaseFunction onEnd{};
  };

  PhaseFunctions phaseFunctions[static_cast<uint8_t>(Phase::Count)] = 
  {
    {.onStart=&App::onJoining, .onIdle=&App::updateJoining},
    {.onStart=&App::onMenu, .onIdle=&App::updateMenu},
    {.onStart=&App::onPaused, .onIdle=&App::updatePaused},
    {.onIdle=&App::updateGame},
  };

  enum class GameType : uint8_t
  {
    Continious,
    SelectGame,
  };

  GameType gameType;

  uint8_t selectedGame{};
  GameState::Difficulty selectedDifficulty{};

  void onJoining()
  {
    auto& state = gameRunner.state;
    for(uint8_t x = 0; x < state.maxPlayerCount; x++)
    {
      display.selectScreenfromIndex(x);

      display.startDraw(0, 4 * 8, Display::Width, 8);
      display.printP("Press button to join"_PSTR);
    }
  }

  void updateJoining()
  {
    auto& state = gameRunner.state;

    for(uint8_t x = 0; x < state.maxPlayerCount; x++)
    {
      if (!state.isPlayerPresent(x) && input.isNewPressed(input.buttonFromIndex(x)))
      {
        state.playerJoin(x);

        display.selectScreenfromIndex(x);
        display.clearRect(0, 4 * 8, Display::Width, 8);
        display.printP("You are player "_PSTR);
        display.print(state.names[x]);

        display.selectMenu();
        display.startDraw(0, 2 * 8, Display::Width, 8);
        display.print('0' + state.playerCount);
        display.printP("/4 player joined"_PSTR);

        display.startDraw(0, 5 * 8, Display::Width, 8);
        display.printP("Press any menu button"_PSTR);

        display.startDraw(0, 6 * 8, Display::Width, 8);
        display.printP("to confirm"_PSTR);
      }
    }

    if(state.playerCount > 0 && input.isNewPressed(Input::Button::MenuButtons))
    {
      setPhase(Phase::Menu);
    }
  }

  void onMenu()
  {
    menu.setMenu(Menus::Main);
  }

  void updateMenu()
  {
    if(auto entry = menu.update(display, input, ledController))
    {
      const auto type = entry->getType();
      if(type == EntryType::Action)
      {
        const auto action = entry->getAction();
        if(action == MenuAction::Play)
        {
          setPhase(Phase::Game);
          gameType = GameType::Continious;
          gameRunner.setGame(0);
        }
      }
      else if(type == EntryType::GameSelect)
      {
          selectedGame = entry->getParam();
      }
      else if(type == EntryType::Difficulty)
      {
          selectedDifficulty = static_cast<GameState::Difficulty>(entry->getParam());
          startSelectedGame();
      }
    }
  }

  void startSelectedGame()
  {
    setPhase(Phase::Game);
    gameType = GameType::SelectGame;
    gameRunner.setGame(selectedGame);
    gameRunner.state.difficulty = selectedDifficulty;
  }

  void startRandomGame()
  {
    setPhase(Phase::Game);
    gameType = GameType::Continious;
    gameRunner.state.difficulty = GameState::Difficulty::None;
    gameRunner.setGame(random(0, GameCount));
  }

  void updateGame()
  {
    gameRunner.update(deltaTime, display, input, ledController);

    if(input.isNewPressed(Input::Button::MenuButtons))
    {
      setPhase(Phase::Paused);
    }

    if(gameRunner.state.phase == GameState::Phase::Finished)
    {
      if(gameType == GameType::Continious)
      {
        startRandomGame();
      }
      else if(gameType == GameType::SelectGame)
      {
        setPhase(Phase::Menu);
      }
    }

    display.selectMenu();
    display.clearRect(Display::Width - (Font::charWidth + 1) * 3, 0, (Font::charWidth + 1) * 3, 8);
    display.print_L(gameRunner.state.deltaTime);
  }

  void onPaused()
  {
    menu.setMenu(Menus::Paused);
  }

  void updatePaused()
  {
    if(auto entry = menu.update(display, input, ledController))
    {
      if(entry->getType() == EntryType::Action)
      {
        if(entry->getAction() == MenuAction::Resume)
        {
          setPhase(Phase::Game);
        }
        else if(entry->getAction() == MenuAction::ExitToMenu)
        {
          setPhase(Phase::Menu);
        }
        else if(entry->getAction() == MenuAction::Traitor)
        {
          
        }
      }
    }
  }

public:

  void init()
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
  }

  void setPhase(Phase newPhase)
  {
    if(const auto onEnd = phaseFunctions[static_cast<uint8_t>(phase)].onEnd)
    {
      (this->*onEnd)();
    }
  
    phase = newPhase;

    display.selectScreen(Display::Screen::All);
    display.clearRect();

    ledController.clear();

    if(const auto onStart = phaseFunctions[static_cast<uint8_t>(phase)].onStart)
    {
      (this->*onStart)();
    }
  }

  void update()
  {
    const auto now = millis();
    deltaTime = now - lastFrame;
    lastFrame = now;

    input.update();

    if(const auto onIdle = phaseFunctions[static_cast<uint8_t>(phase)].onIdle)
    {
      (this->*onIdle)();
    }

    ledController.display();

    //if (myDFPlayer.available()) {
    //  printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
    //}
  }
};
