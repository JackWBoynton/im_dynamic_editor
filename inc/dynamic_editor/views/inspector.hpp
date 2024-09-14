#pragma once

#include <dynamic_editor/nodes/node.hpp>
#include <dynamic_editor/utils/imgui_extras.hpp>

namespace dynamic_editor::views {

class Inspector {
public:
  Inspector(std::shared_ptr<nodes::NodeHolder> &nodes) : m_Nodes(nodes) {}

  void RenderWindowed();
  void Render();

  bool &GetWindowOpen() { return m_WindowOpen; }

private:
  std::shared_ptr<nodes::NodeHolder> m_Nodes;
  bool m_WindowOpen = true;
};

} // namespace dynamic_editor::views
