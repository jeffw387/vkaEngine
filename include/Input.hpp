#pragma once
#include <variant>

#include "spookyhash.hpp"
#include "CircularQueue.hpp"
#include "Engine.hpp"

namespace Input {

struct Signature {
  int code;
  int action;
};

template <typename T>
struct Event {
  Signature signature;
  vka::Clock::time_point eventTime;
};

inline bool operator==(Signature a, Signature b) {
  return a.code == b.code && a.action == b.action;
}

struct Key;
struct Mouse;
using InputEvent = std::variant<Event<Key>, Event<Mouse>>;
class Manager {
public:
  void addInputToQueue(InputEvent inputEvent) {
    inputQueue.pushLast(inputEvent);
  };
  auto getEventBefore(vka::Clock::time_point cutoff) {
    return inputQueue.popFirstIf([=](auto inputEvent) {
      return std::visit(
          [=](const auto& inputVariant) {
            return inputVariant.eventTime < cutoff;
          },
          inputEvent);
    });
  }

private:
  CircularQueue<InputEvent, 256> inputQueue;
};
}  // namespace Input

namespace std {
template <>
struct hash<Input::Signature> {
  size_t operator()(Input::Signature value) const {
    return SpookyHash::Hash64(&value, sizeof(Input::Signature), 0);
  }
};
}  // namespace std