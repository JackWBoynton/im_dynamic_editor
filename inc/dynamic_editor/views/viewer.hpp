#pragma once

#include <dynamic_editor/nodes/node.hpp>

namespace dynamic_editor::views {

class Viewer {
public:
  Viewer(std::shared_ptr<nodes::NodeHolder> &nodes) : m_Nodes(nodes) {}

  void RenderWindowed(bool &show);
  void Render();

private:
  std::shared_ptr<nodes::NodeHolder> m_Nodes;
};

} // namespace dynamic_editor::views
