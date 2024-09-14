#include <dynamic_editor/api/dynamic_editor.hpp>

#include <dynamic_editor/nodes/attribute.hpp>
#include <dynamic_editor/nodes/link.hpp>
#include <dynamic_editor/nodes/node.hpp>
#include <dynamic_editor/views/editor.hpp>

#include <memory>
#include <string>

#include "imgui.h"

namespace dynamic_editor::views {
void Editor::RenderWindowed() {
  if (ImGui::Begin("Dynamic Editor Editor", nullptr, ImGuiWindowFlags_MenuBar))
    Render();
  ImGui::End();
}

void Editor::Render() {
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Save Nodes", "", nullptr)) {
        // auto content = DumpNodes();
        // auto content_str = content.dump(2);
        //  fs::OpenFileBrowser(fs::DialogMode::Save, {{"JSON Files", "json"}},
        //                      [content_str](std::filesystem::path const &path)
        //                      {
        //                        fs::FsWriteString(path, content_str);
        //                      });
      }
      if (ImGui::MenuItem("Load Nodes", "", nullptr)) {
        // fs::OpenFileBrowser(
        //     fs::DialogMode::Open, {{"JSON Files", "json"}},
        //     [this](std::filesystem::path const &path) { LoadNodes(path); });
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  ImNodes::SetCurrentContext(m_Context.get());

  // adding nodes and such
  DrawContextMenus();

  auto size = ImGui::GetContentRegionAvail();
  size.y -= (ImGui::GetTextLineHeightWithSpacing() * 1.3F);

  if (ImGui::BeginChild("##node_editor", size)) {
    ImNodes::BeginNodeEditor();

    if (m_NewDroppedNodeId > 0) {
      ImNodes::SetNodeScreenSpacePos(m_NewDroppedNodeId,
                                     m_NewDroppedNodeMousePos);
      m_NewDroppedNodeId = -1;
    }
    bool const still_updating = m_UpdateNodePositions;
    if (m_Nodes) {
      for (auto &node : *m_Nodes.get()) {
        ImNodes::SnapNodeToGrid(node->GetId());

        bool const has_error =
            node->GetState() != nodes::NodeState_OK ||
            (m_CurrNodeError.has_value() &&
             m_CurrNodeError->NodePtr->GetId() == node->GetId());
        if (has_error) {
          ImNodes::PushColorStyle(ImNodesCol_NodeOutline, 0xFF0000FF);
        }

        DrawNode(*node);

        if (has_error) {
          ImNodes::PopColorStyle();
        }
      }
      // render links
      for (auto const &link : m_Links) {
        ImNodes::Link(link.GetId(), link.GetFromId(), link.GetToId());
      }
    }
    ImNodes::MiniMap(0.2F, ImNodesMiniMapLocation_BottomRight);
    ImNodes::EndNodeEditor();
  }
  ImGui::EndChild();
  if (ImGui::BeginDragDropTarget()) {
    if (ImGuiPayload const *payload =
            ImGui::AcceptDragDropPayload("INPUT DATA SOURCE")) {
      // TODO: generalize
      // auto node =
      //     std::make_unique<DataSourceNode>(dropped_channel.ChannelId,
      //     GetTag());
      // m_Nodes.push_back(std::move(node));
      // m_NewDroppedNodeMousePos = ImGui::GetMousePos();
      // m_NewDroppedNodeId = m_Nodes.back()->GetId();
    }
    ImGui::EndDragDropTarget();
  }
}

void Editor::EraseLink(int id) {
  auto link = std::find_if(m_Links.begin(), m_Links.end(),
                           [&id](auto link) { return link.GetId() == id; });

  if (link == m_Links.end()) {
    return;
  }

  for (auto &node : *m_Nodes) {
    for (auto &attribute : node->GetAttributes())
      attribute.RemoveConnectedAttribute(id);
  }
  m_Links.erase(link);
}

void Editor::EraseNodes(std::vector<int> const &ids) {
  // Loop over the IDs of all nodes that should be removed
  for (int id : ids) {
    auto node =
        std::find_if(m_Nodes->begin(), m_Nodes->end(),
                     [&id](auto const &node) { return node->GetId() == id; });

    for (auto &attr : (*node)->GetAttributes()) {
      std::vector<int> links_to_remove;

      for (auto &[linkId, connectedAttr] : attr.GetConnectedAttributes())
        links_to_remove.push_back(linkId);

      for (auto link_id : links_to_remove)
        EraseLink(link_id);
    }
  }

  for (int id : ids) {
    auto node =
        std::find_if(m_Nodes->begin(), m_Nodes->end(),
                     [&id](auto const &node) { return node->GetId() == id; });

    std::erase_if(m_EndNodes,
                  [&id](auto const &node) { return node->GetId() == id; });

    m_Nodes->erase(node);
  }
}

void Editor::DrawNode(nodes::Node &node) {

  // If a Node position update is pending, update the Node position
  int const node_id = node.GetId();
  if (m_UpdateNodePositions) {
    ImNodes::SetNodeGridSpacePos(node_id, node.GetPosition());
  } else {
    if (ImNodes::ObjectPoolFind(ImNodes::EditorContextGet().Nodes, node_id) >=
        0) {
      node.SetPosition(ImNodes::GetNodeGridSpacePos(node_id));
    }
  }

  ImNodes::BeginNode(node_id);
  {
    ImNodes::BeginNodeTitleBar();
    ImGui::TextUnformatted(node.GetTitle().c_str());
    ImNodes::EndNodeTitleBar();

    ImGui::PopStyleVar();
    node.WrapDrawNode();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0F, 1.0F));

    for (auto &attribute : node.GetAttributes()) {
      ImNodesPinShape pin_shape = 0;

      auto attribute_type = attribute.GetType();
      switch (attribute_type) {
      default:
      case nodes::Attribute::Type::Float:
        pin_shape = ImNodesPinShape_Circle;
        break;
      case nodes::Attribute::Type::Boolean:
        pin_shape = ImNodesPinShape_Triangle;
        break;
      case nodes::Attribute::Type::Int:
        pin_shape = ImNodesPinShape_Quad;
        break;
      }

      if (attribute.GetIo() == nodes::Attribute::IO::In) {
        ImNodes::BeginInputAttribute(attribute.GetId(), pin_shape);
        ImGui::TextUnformatted(attribute.GetName().c_str());
        ImNodes::EndInputAttribute();
      } else if (attribute.GetIo() == nodes::Attribute::IO::Out) {
        ImNodes::BeginOutputAttribute(attribute.GetId(),
                                      ImNodesPinShape(pin_shape + 1));
        ImGui::TextUnformatted(attribute.GetName().c_str());
        ImNodes::EndOutputAttribute();
      }
    }
  }

  ImNodes::EndNode();
  ImNodes::SetNodeGridSpacePos(node_id, node.GetPosition());
}

void Editor::DrawContextMenus() {
  if (ImGui::IsMouseDown(ImGuiMouseButton_Right) &&
      ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) {
    ImNodes::ClearNodeSelection();
    ImNodes::ClearLinkSelection();

    m_RightClickedCoords = ImGui::GetMousePos();

    if (ImNodes::IsNodeHovered(&m_RightClickedId))
      ImGui::OpenPopup("Node Menu");
    else if (ImNodes::IsLinkHovered(&m_RightClickedId))
      ImGui::OpenPopup("Link Menu");
    else
      ImGui::OpenPopup("Context Menu");
  }

  // main context menu
  if (ImGui::BeginPopup("Context Menu")) {
    std::shared_ptr<nodes::Node> node;

    if (ImNodes::NumSelectedNodes() > 0 || ImNodes::NumSelectedLinks() > 0) {
      if (ImGui::MenuItem("Remove Selection")) {
        // delete nodes
        {
          std::vector<int> ids;
          ids.resize(ImNodes::NumSelectedNodes());
          ImNodes::GetSelectedNodes(ids.data());

          EraseNodes(ids);
          ImNodes::ClearNodeSelection();
        }

        // delete links
        {
          std::vector<int> ids;
          ids.resize(ImNodes::NumSelectedLinks());
          ImNodes::GetSelectedLinks(ids.data());

          for (auto id : ids)
            EraseLink(id);
          ImNodes::ClearLinkSelection();
        }
      }
    }

    for (auto const &[cat, name, desc, function] : api::GetNodeFactories()) {
      if (name.empty() && desc.empty()) {
        ImGui::Separator();
      } else if (cat.empty()) {
        if (ImGui::MenuItem(name.c_str())) {
          node = function();
        }
        if (!desc.empty() && ImGui::BeginItemTooltip()) {
          ImGui::Text("%s", desc.c_str());
          ImGui::EndTooltip();
        }
      } else {
        if (ImGui::BeginMenu(cat.c_str())) {
          if (ImGui::MenuItem(name.c_str())) {
            node = function();
          }
          if (!desc.empty() && ImGui::BeginItemTooltip()) {
            ImGui::Text("%s", desc.c_str());
            ImGui::EndTooltip();
          }
          ImGui::EndMenu();
        }
      }
    }

    if (node != nullptr) {
      bool has_output = false;
      bool has_input = false;
      for (auto &attr : node->GetAttributes()) {
        switch (attr.GetIo()) {
        case nodes::Attribute::IO::In:
          has_input = true;
          break;
        case nodes::Attribute::IO::Out:
          has_output = true;
          break;
        }
      }

      if (has_input && !has_output) {
        m_EndNodes.push_back(node.get());
      }
      ImNodes::SetNodeScreenSpacePos(node->GetId(), m_RightClickedCoords);
      node->SetPosition(m_RightClickedCoords);
      if (!m_Nodes)
        throw std::runtime_error("Nodes is null!");
      m_Nodes->push_back(node);
    }

    ImGui::EndPopup();
  }

  // Node menu
  if (ImGui::BeginPopup("Node Menu")) {
    if (ImGui::MenuItem("Remove Node")) {
      this->EraseNodes({m_RightClickedId});
    }

    ImGui::EndPopup();
  }

  // link menu
  if (ImGui::BeginPopup("Link Menu")) {
    if (ImGui::MenuItem("Remove Link")) {
      this->EraseLink(m_RightClickedId);
    }

    ImGui::EndPopup();
  }
}

} // namespace dynamic_editor::views
