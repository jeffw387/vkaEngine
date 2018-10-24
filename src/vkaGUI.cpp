#include "vkaGUI.hpp"
#include <cstring>

namespace vka {
GUI::GUI(VkImage fontImage) {
  ImGui::CreateContext();

  getIO().Fonts->TexID = fontImage;
}

void GUI::newFrame() { ImGui::NewFrame(); }

ImDrawData* GUI::render() {
  ImGui::EndFrame();
  ImGui::Render();
  return ImGui::GetDrawData();
}

ImGuiIO& GUI::getIO() { return ImGui::GetIO(); }

GUI::~GUI() { ImGui::DestroyContext(); }
}  // namespace vka