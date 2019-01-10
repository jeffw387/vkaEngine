#pragma once

template <typename T>
struct move_into{
  move_into(T& valueReference) : m_valueReference(valueReference) {};
  operator()(T value) {
    m_valueReference = std::move(value);
  }
private:
  T& m_valueReference;
};