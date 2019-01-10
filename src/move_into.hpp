#pragma once

template <typename T>
struct move_into{
  move_into(T& valueReference) : m_destinationReference(valueReference) {};
  void operator()(T destination) {
    m_destinationReference = std::move(destination);
  }
private:
  T& m_destinationReference;
};

template <typename T>
struct move_value_into {
  move_value_into(T value, T& destination) { destination = std::move(value); }
  void operator()() {}
};