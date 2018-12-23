#pragma once
#include <variant>
#include <map>

#include "spookyhash.hpp"
#include "FlatList.hpp"
#include "Clock.hpp"

namespace Input {

struct Action {
  std::string name;
};

inline bool operator==(Action a, Action b) { return a.name == b.name; }

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
    inputQueue.push_last(inputEvent);
  };
  auto getEventBefore(vka::Clock::time_point cutoff) {
    if (auto eventOptional = inputQueue.first_if([=](auto inputEvent) {
      return std::visit(
          [=](const auto& inputVariant) {
            return inputVariant.eventTime < cutoff;
          },
          inputEvent);
        })) {
      inputQueue.pop_first();
      return eventOptional;
    }
  }

private:
  FlatList<InputEvent, 256> inputQueue;
};

using Bindings = std::unordered_multimap<Action, Signature>;
using InverseBindings = std::unordered_map<Signature, Action>;
using State = std::unordered_map<Action, bool>;
}  // namespace Input

namespace std {
template <>
struct hash<Input::Signature> {
  size_t operator()(Input::Signature value) const {
    return SpookyHash::Hash64(&value, sizeof(Input::Signature), 0);
  }
};

template <>
struct hash<Input::Action> {
  size_t operator()(Input::Action value) const {
    return SpookyHash::Hash64(&value, sizeof(Input::Action), 0);
  }
};
}  // namespace std