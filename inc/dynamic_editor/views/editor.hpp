#pragma once

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include <dynamic_editor/nodes/link.hpp>
#include <dynamic_editor/nodes/node.hpp>

#include "imnodes.h"
#include "imnodes_internal.h"

namespace dynamic_editor::views {

class Editor {
public:
  Editor(std::shared_ptr<nodes::NodeHolder> &nodes) : m_Nodes(nodes) {}

  void RenderWindowed();
  void Render();

private:
  void DrawContextMenus();
  void DrawNode(nodes::Node &node);
  void EraseNodes(const std::vector<int> &ids);
  void EraseLink(int id);

  std::unique_ptr<ImNodesContext, void (*)(ImNodesContext *)> m_Context = {
      [] {
        ImNodesContext *ctx = ImNodes::CreateContext();
        ctx->Style = ImNodes::GetStyle();
        ctx->Io = ImNodes::GetIO();
        ctx->AttributeFlagStack = GImNodes->AttributeFlagStack;

        return ctx;
      }(),
      ImNodes::DestroyContext};

  std::shared_ptr<nodes::NodeHolder> m_Nodes;
  std::list<nodes::Node *> m_EndNodes;
  std::list<nodes::Link> m_Links;
  int m_RightClickedId = -1;
  bool m_UpdateNodePositions = false;
  ImVec2 m_RightClickedCoords;
  int m_NewDroppedNodeId = -1;
  ImVec2 m_NewDroppedNodeMousePos;

  std::optional<nodes::Node::NodeError> m_CurrNodeError;
};

} // namespace dynamic_editor::views
