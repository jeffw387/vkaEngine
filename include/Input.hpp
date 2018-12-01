#pragma once
#include <variant>

#include "CircularQueue.hpp"
#include "Engine.hpp"

namespace Input {
struct KeyEvent {
  int keyCode;
  int action;
  vka::Clock::time_point eventTime;
};

struct MouseEvent {
  int buttonCode;
  int action;
  vka::Clock::time_point eventTime;
};

using InputEvent = std::variant<KeyEvent, MouseEvent>;
class Manager {
public:
  void addInputToQueue(InputEvent inputEvent) {
    inputQueue.pushLast(inputEvent);
  };
  auto getEventBefore(vka::Clock::time_point cutoff) {
     return inputQueue.popFirstIf([](auto inputEvent){ return inputEvent.eventTime < cutoff;});
  }
private:
  CircularQueue<InputEvent, 256> inputQueue;
};
}