#include <dynamic_editor/views/editor.hpp>

#include "imgui.h"

namespace dynamic_editor::views {

void Editor::Render() { ImGui::Text("Hello, world!"); }

void Editor::RenderWindowed() {
  if (ImGui::Begin("Dynamic Editor Editor"))
    Render();
  ImGui::End();
}

} // namespace dynamic_editor::views
