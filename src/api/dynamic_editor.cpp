#include <dynamic_editor/api/dynamic_editor.hpp>
#include <dynamic_editor/views/viewer.hpp>

#include "imgui.h"
#include "imgui_internal.h"

namespace dynamic_editor::api {

void DynamicEditor::RenderWindowed() {
  if (ImGui::Begin("Dynamic Editor")) {

    ConfigureDockspace();
    ImGui::DockSpace(m_dockspace_id, ImVec2(-1, -1), ImGuiDockNodeFlags_None,
                     &m_dockspace_wc);
    m_viewer.RenderWindowed();
    m_editor.RenderWindowed();
  }
  ImGui::End();
}

void DynamicEditor::ConfigureDockspace() {
  if (m_dockspace_id == 0)
    m_dockspace_id = ImGui::GetID("Dynamic Editor");

  if (ImGui::DockBuilderGetNode(m_dockspace_id) == nullptr) {
    ImGui::DockBuilderRemoveNode(m_dockspace_id);
    ImGui::DockBuilderAddNode(m_dockspace_id, ImGuiDockNodeFlags_None);

    auto dock_main_id = m_dockspace_id;
    auto dock_id_left = ImGui::DockBuilderSplitNode(
        dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);
    auto dock_id_right = ImGui::DockBuilderSplitNode(
        dock_main_id, ImGuiDir_Right, 0.2f, nullptr, &dock_main_id);

    ImGui::DockBuilderDockWindow("Dynamic Editor Editor", dock_id_left);
    ImGui::DockBuilderDockWindow("Dynamic Editor Viewer", dock_id_right);
    ImGui::DockBuilderFinish(m_dockspace_id);
  }
}

} // namespace dynamic_editor::api
