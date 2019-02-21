#pragma once
#include <map>
#include <unordered_map>
#include <variant>

#include "clock.hpp"
#include "flat_list.hpp"
#include "spookyhash.hpp"
#include "surface.hpp"

namespace input {

struct action {
  std::string name;
};

inline bool operator==(action a, action b) {
  return a.name == b.name;
}

struct signature {
  int code;
  int action;
};

template <typename T>
struct event {
  signature eventSignature;
  vka::Clock::time_point eventTime;
};

inline bool operator==(signature a, signature b) {
  return a.code == b.code && a.action == b.action;
}

struct key;
struct mouse;
using input_event = std::variant<event<key>, event<mouse>>;

struct cursor_position {
  double x = {};
  double y = {};
};

class manager {
public:
  manager(vka::WindowType*);
  std::optional<input_event> next_event_before(
      vka::Clock::time_point cutoff) {
    auto eventSelect = [=](auto inputEvent) {
      return std::visit(
          [=](const auto& inputVariant) {
            return inputVariant.eventTime < cutoff;
          },
          inputEvent);
    };

    if (auto eventOptional =
            m_inputQueue.first_if(eventSelect)) {
      m_inputQueue.pop_first();
      return eventOptional;
    }
    return {};
  }

  platform::window_should_close poll_events(
      vka::WindowType* window) {
    return platform::glfw::poll_os(window);
  }

  void enqueue(input_event inputEvent) {
    m_inputQueue.push_last(inputEvent);
  };

  void update_cursor_position(cursor_position position) {
    m_position = position;
  }

  cursor_position current_cursor_position() const noexcept {
    return m_position;
  }

private:
  FlatList<input_event, 256> m_inputQueue;
  cursor_position m_position;
};

using bindings = std::unordered_multimap<action, signature>;
using inverse_bindings =
    std::unordered_map<signature, action>;
using state = std::unordered_map<action, bool>;
}  // namespace input

namespace std {
template <>
struct hash<input::signature> {
  size_t operator()(input::signature value) const {
    return SpookyHash::Hash64(
        &value, sizeof(input::signature), 0);
  }
};

template <>
struct hash<input::action> {
  size_t operator()(input::action value) const {
    return SpookyHash::Hash64(
        &value, sizeof(input::action), 0);
  }
};
}  // namespace std