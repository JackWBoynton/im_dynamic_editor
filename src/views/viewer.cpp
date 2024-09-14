#include <dynamic_editor/views/viewer.hpp>

#include "imgrid.h"
#include "imgui.h"

namespace dynamic_editor::views {

void Viewer::Render() {
  if (!m_Nodes)
    return;
  for (auto &node : *m_Nodes)
    node->DrawViewerNode();
}

void Viewer::RenderWindowed() {
  ImGrid::PushStyleVar(ImGridStyleVar_GridSpacing, 70.0f);
  if (ImGui::Begin("Dynamic Editor Viewer")) {
    ImGrid::BeginGrid();
    { Render(); }
    ImGrid::EndGrid();
  }
  ImGui::End();
}

} // namespace dynamic_editor::views
