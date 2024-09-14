#include <dynamic_editor/utils/imgui_extras.hpp>
#include <dynamic_editor/views/inspector.hpp>

#include "imgrid.h"
#include "imgui.h"

namespace dynamic_editor::views {

void Inspector::Render() {
  if (!m_Nodes)
    return;

  for (auto &node : m_Nodes->SelectedNodes) {
    node->DrawProperties();
  }
}

void Inspector::RenderWindowed() {
  if (ImGui::Begin("Dynamic Editor Inspector")) {
    ImGui::Text("Inspector");
    Render();
  }
  ImGui::End();
}

} // namespace dynamic_editor::views
