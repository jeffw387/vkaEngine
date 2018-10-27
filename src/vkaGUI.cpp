#include "vkaGUI.hpp"
#include <cstring>

namespace vka {
GUI::GUI() {
  ImGui::CreateContext();
}

GUI::~GUI() { ImGui::DestroyContext(); }
}  // namespace vka