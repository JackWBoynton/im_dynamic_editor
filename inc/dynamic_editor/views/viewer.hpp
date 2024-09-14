#pragma once

#include <dynamic_editor/nodes/node.hpp>

namespace dynamic_editor::views {

class Viewer {
public:
  Viewer(nodes::NodeHolder nodes) : m_Nodes(nodes) {}

  void RenderWindowed();
  void Render();

private:
  nodes::NodeHolder m_Nodes;
};

} // namespace dynamic_editor::views
