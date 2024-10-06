#include <dynamic_editor/views/viewer.hpp>

#include "imgrid.h"
#include "imgui.h"

namespace dynamic_editor::views {

void Viewer::Render() {
  for (auto &node : m_Nodes->Nodes) {
    if (!node->ShouldRenderViewer())
      continue;
    ImGrid::BeginEntry(node->GetId());
    {
      if (node->ShouldRenderTitleBar()) {
        ImGrid::BeginEntryTitleBar();
        ImGui::Text("%s", node->GetTitle().c_str());
        node->RenderErrors();
        ImGrid::EndEntryTitleBar();
      }

      if (!node->GetHasError())
        node->DrawViewerNodeContent();
    }
    ImGrid::EndEntry();

    node->ClearError();
    node->ClearWarning();
  }
}

void Viewer::RenderWindowed(bool &show) {
  if (!show)
    return;

  for (auto &node : m_Nodes->Nodes) {
    if (ImGrid::IsNodeSelected(node->GetId()))
      m_Nodes->SelectNode(node->GetId());
  }

  ImGrid::PushStyleVar(ImGridStyleVar_GridSpacing, 70.0f);
  if (ImGui::Begin("Dynamic Editor Viewer", &show)) {
    ImGrid::BeginGrid();
    { Render(); }
    ImGrid::EndGrid();
  }
  ImGui::End();
}

} // namespace dynamic_editor::views
