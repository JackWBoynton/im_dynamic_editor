#include <dynamic_editor/views/viewer.hpp>

#include "imgui.h"

namespace dynamic_editor::views {

void Viewer::Render() { ImGui::Text("Hello, world!"); }

void Viewer::RenderWindowed() {
  if (ImGui::Begin("Dynamic Editor Viewer"))
    Render();
  ImGui::End();
}

} // namespace dynamic_editor::views
