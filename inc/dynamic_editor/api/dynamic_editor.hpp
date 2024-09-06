#pragma once

#include <dynamic_editor/views/editor.hpp>
#include <dynamic_editor/views/viewer.hpp>

#include "imgui.h"
#include "imgui_internal.h"

namespace dynamic_editor::api {

class DynamicEditor {
public:
  DynamicEditor() = default;

  void Render();
  void RenderWindowed();

private:
  void ConfigureDockspace();

  ImGuiID m_dockspace_id;
  ImGuiWindowClass m_dockspace_wc;
  ImGuiWindowClass m_dockspace_sub_wc;

  views::Editor m_editor;
  views::Viewer m_viewer;
};

} // namespace dynamic_editor::api
