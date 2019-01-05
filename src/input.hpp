#pragma once
#include <variant>
#include <map>

#include "spookyhash.hpp"
#include "FlatList.hpp"
#include "Clock.hpp"

namespace input {

struct action {
  std::string name;
};

inline bool operator==(action a, action b) { return a.name == b.name; }

struct signature {
  int code;
  int action;
};

template <typename T>
struct event {
  signature signature;
  vka::Clock::time_point eventTime;
};

inline bool operator==(signature a, signature b) {
  return a.code == b.code && a.action == b.action;
}

struct key;
struct mouse;
using input_event = std::variant<event<key>, event<mouse>>;
class manager {
public:
  void enqueue(input_event inputEvent) {
    m_inputQueue.push_last(inputEvent);
  };
  auto next_event_before(vka::Clock::time_point cutoff) {
    if (auto eventOptional = m_inputQueue.first_if([=](auto inputEvent) {
          return std::visit(
              [=](const auto& inputVariant) {
                return inputVariant.eventTime < cutoff;
              },
              inputEvent);
        })) {
      m_inputQueue.pop_first();
      return eventOptional;
    }
  }

private:
  FlatList<input_event, 256> m_inputQueue;
};

using bindings = std::unordered_multimap<action, signature>;
using inverse_bindings = std::unordered_map<signature, action>;
using state = std::unordered_map<action, bool>;
}  // namespace input

namespace std {
template <>
struct hash<input::signature> {
  size_t operator()(input::signature value) const {
    return SpookyHash::Hash64(&value, sizeof(input::signature), 0);
  }
};

template <>
struct hash<input::action> {
  size_t operator()(input::action value) const {
    return SpookyHash::Hash64(&value, sizeof(input::action), 0);
  }
};
}  // namespace std