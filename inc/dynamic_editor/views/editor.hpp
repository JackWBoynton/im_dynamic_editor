#pragma once

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <thread>
#include <vector>

#include <dynamic_editor/nodes/link.hpp>
#include <dynamic_editor/nodes/node.hpp>

#include "imnodes.h"
#include "imnodes_internal.h"

#include <nlohmann/json.hpp>

namespace dynamic_editor::views {

class Editor {
public:
  Editor(std::shared_ptr<nodes::NodeHolder> &nodes) : m_Nodes(nodes) {}

  void RenderWindowed(bool &show);
  void Render();

  std::shared_ptr<nodes::Node> LoadNode(const nlohmann::json &data);
  void LoadNodes(const nlohmann::json &data);
  nlohmann::json DumpNode(nodes::Node *node) const;
  nlohmann::json DumpNodes() const;

private:
  void DrawContextMenus();
  void DrawNode(nodes::Node &node);
  void EraseNodes(const std::vector<int> &ids);
  void EraseLink(int id);

  void ProcessNodes();

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

  std::thread m_thread;
  bool m_continuousProcessing = false;
};

} // namespace dynamic_editor::views
