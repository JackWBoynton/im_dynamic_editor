#include <dynamic_editor/api/dynamic_editor.hpp>
#include <dynamic_editor/views/viewer.hpp>

#include "imgui.h"
#include "imgui_internal.h"

#include <set>

namespace dynamic_editor::api {

namespace impl {
static std::vector<nodes::NodeFactory> s_factories{};
void RegisterNodeType(const nodes::NodeFactory &factory) {
  s_factories.push_back(factory);
}

} // namespace impl

const std::vector<nodes::NodeFactory> &GetNodeFactories() {
  return impl::s_factories;
}

void DynamicEditor::RenderWindowed() {
  ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Dynamic Editor", nullptr, ImGuiWindowFlags_MenuBar)) {
    Render();
  }
  ImGui::End();
}

void DynamicEditor::Render() {
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("View")) {
      if (ImGui::MenuItem("Editor Open", "", m_show_editor)) {
        m_show_editor = !m_show_editor;
      }
      if (ImGui::MenuItem("Viewer Open", "", m_show_viewer)) {
        m_show_viewer = !m_show_viewer;
      }
      if (ImGui::MenuItem("Inspector Open", "", m_show_inspector)) {
        m_show_inspector = !m_show_inspector;
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
  m_nodes->ResetSelectedNodes();
  ConfigureDockspace();
  ImGui::DockSpace(m_dockspace_id, ImVec2(-1, -1), ImGuiDockNodeFlags_None,
                   &m_dockspace_wc);
  m_viewer.RenderWindowed(m_show_viewer);
  m_editor.RenderWindowed(m_show_editor);
  m_inspector.RenderWindowed(m_show_inspector);
}

void DynamicEditor::ConfigureDockspace() {
  if (m_dockspace_id == 0)
    m_dockspace_id = ImGui::GetID("Dynamic Editor");

  if (ImGui::DockBuilderGetNode(m_dockspace_id) == nullptr) {
    ImGui::DockBuilderRemoveNode(m_dockspace_id);
    ImGui::DockBuilderAddNode(m_dockspace_id, ImGuiDockNodeFlags_None);

    // 3 columns 0.4, 0.4, 0.2

    auto dock_main_id = m_dockspace_id;
    auto dock_id_left = ImGui::DockBuilderSplitNode(
        dock_main_id, ImGuiDir_Left, 0.4, nullptr, &dock_main_id);
    auto dock_id_center = ImGui::DockBuilderSplitNode(
        dock_main_id, ImGuiDir_Left, 0.4, nullptr, &dock_main_id);
    auto dock_id_right = dock_main_id;

    ImGui::DockBuilderDockWindow("Dynamic Editor Editor", dock_id_left);
    ImGui::DockBuilderDockWindow("Dynamic Editor Viewer", dock_id_center);
    ImGui::DockBuilderDockWindow("Dynamic Editor Inspector", dock_id_right);
    ImGui::DockBuilderFinish(m_dockspace_id);
  }
}

} // namespace dynamic_editor::api
