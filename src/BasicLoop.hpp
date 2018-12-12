#pragma once
#include <functional>

class BasicLoop {
  std::function<bool()> callback = {};

public:
  BasicLoop(){};
  BasicLoop(std::function<bool()> callback) : callback(callback) {}
  void run() const {
    while (true) {
      if (callback) {
        auto continueLoop = callback();
        if (!continueLoop) {
          break;
        }
      }
    }
  }
};