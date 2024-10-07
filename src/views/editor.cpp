#include <dynamic_editor/api/dynamic_editor.hpp>

#include <dynamic_editor/nodes/attribute.hpp>
#include <dynamic_editor/nodes/link.hpp>
#include <dynamic_editor/nodes/node.hpp>
#include <dynamic_editor/views/editor.hpp>
#include <dynamic_editor/views/inspector.hpp>

#include "codicons_internal.hpp"

#include <nlohmann/json.hpp>

#include <memory>
#include <string>

#include "imgui.h"

namespace dynamic_editor::views {
void Editor::RenderWindowed(bool &show) {
  if (!show)
    return;
  if (ImGui::Begin("Dynamic Editor Editor", &show, ImGuiWindowFlags_MenuBar))
    Render();
  ImGui::End();
}

void Editor::LoadNodes(const nlohmann::json &data) {
  m_Nodes->Nodes.clear();
  m_EndNodes.clear();
  m_Links.clear();
  printf("loading nodes from %s\n", data.dump(4).c_str());

  try {
    if (data.contains("nodes")) {
      auto nodes_data = data["nodes"];
      for (const auto &node_data : nodes_data) {
        auto new_node = LoadNode(node_data);
        if (new_node == nullptr)
          continue;

        bool hasOutput = false;
        bool hasInput = false;
        uint32_t attrIndex = 0;
        for (auto &attr : new_node->GetAttributes()) {
          if (attr.GetIo() == nodes::Attribute::IO::Out)
            hasOutput = true;

          if (attr.GetIo() == nodes::Attribute::IO::In)
            hasInput = true;

          if (attrIndex < node_data["attrs"].size())
            attr.SetId(node_data["attrs"][attrIndex]);
          else
            attr.SetId(-1);

          attrIndex++;
        }

        if (hasInput && !hasOutput)
          m_EndNodes.push_back(new_node.get());

        new_node->SetPosition(
            ImVec2(node_data["pos"]["x"], node_data["pos"]["y"]));

        m_Nodes->Nodes.push_back(std::move(new_node));
      }
    }

    int maxLinkId = 0;
    if (data.contains("links")) {
      for (auto &link : data["links"]) {
        nodes::Link newLink(link["from"], link["to"]);

        int linkId = link["id"];
        maxLinkId = std::max(linkId, maxLinkId);

        newLink.SetId(linkId);
        m_Links.push_back(newLink);

        nodes::Attribute *fromAttr = nullptr, *toAttr = nullptr;
        for (auto &node : m_Nodes->Nodes) {
          for (auto &attribute : node->GetAttributes()) {
            if (attribute.GetId() == newLink.GetFromId())
              fromAttr = &attribute;
            else if (attribute.GetId() == newLink.GetToId())
              toAttr = &attribute;
          }
        }

        if (fromAttr == nullptr || toAttr == nullptr)
          continue;

        if (fromAttr->GetType() != toAttr->GetType())
          continue;

        if (fromAttr->GetIo() == toAttr->GetIo())
          continue;

        if (!toAttr->GetConnectedAttributes().empty())
          continue;

        fromAttr->AddConnectedAttribute(newLink.GetId(), toAttr);
        toAttr->AddConnectedAttribute(newLink.GetId(), fromAttr);
      }
    }

    int maxNodeId = 0;
    int maxAttrId = 0;
    for (auto &node : m_Nodes->Nodes) {
      maxNodeId = std::max(maxNodeId, node->GetId());

      for (auto &attr : node->GetAttributes()) {
        maxAttrId = std::max(maxAttrId, attr.GetId());
      }
    }

    for (auto &node : m_Nodes->Nodes) {
      if (node->GetId() == -1) {
        maxNodeId += 1;
        node->SetId(maxNodeId);
      }
    }

    nodes::Node::SetIdCounter(maxNodeId + 1);
    nodes::Attribute::SetIdCounter(maxAttrId + 1);
    nodes::Link::SetIdCounter(maxLinkId + 1);

    m_UpdateNodePositions = true;
  } catch (nlohmann::json::exception const &e) {
    printf("Error loading nodes: %s\n", e.what());
  }
}

std::shared_ptr<nodes::Node> Editor::LoadNode(const nlohmann::json &data) {
  std::shared_ptr<nodes::Node> new_node = nullptr;
  try {
    for (const auto &node_factory : api::GetNodeFactories()) {
      if (data.contains("name") &&
          data["name"].get<std::string>() == node_factory.Name) {
        new_node = node_factory.Func();
      }
    }

    if (data.contains("id"))
      new_node->SetId(data["id"].get<int>());
    if (data.contains("title"))
      new_node->SetTitle(data["title"].get<std::string>());
    uint32_t attrIndex = 0;
    for (auto &attr : new_node->GetAttributes()) {
      if (attrIndex < data["attrs"].size())
        attr.SetId(data["attrs"][attrIndex]);
      else
        attr.SetId(-1);

      attrIndex++;
    }

    if (!data["impl"].is_null())
      new_node->Load(data["impl"]);

  } catch (nlohmann::json::exception const &e) {
    printf("Failed to create a new node from json with %s\n", e.what());
  }

  if (new_node == nullptr)
    printf("Failed to create a new node from json");

  return new_node;
}

nlohmann::json Editor::DumpNode(nodes::Node *node) const {
   
    nlohmann::json output;

    output["title"] = node->GetTitle();
    output["name"] = node->GetName();

    nlohmann::json nodeData;
    node->Dump(nodeData);
    output["impl"] = nodeData;

    output["attrs"] = nlohmann::json::array();
    uint32_t attrIndex = 0;
    for (const auto &attr : node->GetAttributes()) {
      output["attrs"][attrIndex] = attr.GetId();
      attrIndex++;
    }

    return output;
  }

  nlohmann::json Editor::DumpNodes() const {
    nlohmann::json output;

    output["nodes"] = nlohmann::json::array();
    for (auto &node : m_Nodes->Nodes) {
      auto id = node->GetId();
      auto &currNodeOutput = output["nodes"].emplace_back();
      auto pos = node->GetPosition();

      currNodeOutput = DumpNode(node.get());
      currNodeOutput["id"] = id;
      currNodeOutput["pos"] = {{"x", pos.x}, {"y", pos.y}};
    }

    output["links"] = nlohmann::json::array();
    for (auto &link : m_Links) {
      auto id = link.GetId();
      auto &currOutput = output["links"].emplace_back();

      currOutput["id"] = id;
      currOutput["from"] = link.GetFromId();
      currOutput["to"] = link.GetToId();
    }

    return output;
  }

  void Editor::Render() {
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Save Nodes", "", nullptr)) {
          auto content = DumpNodes();
          auto content_str = content.dump(4);
          OnDumpNodes(content_str);
        }
        if (ImGui::MenuItem("Load Nodes", "", nullptr)) {
          LoadNodes(LoadNodesRequested());
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
        for (auto &node : m_Nodes->Nodes) {
          node->CheckForErrors();
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

    int *num_selected_nodes = new int(ImNodes::NumSelectedNodes());
    ImNodes::GetSelectedNodes(num_selected_nodes);
    for (int i = 0; i < *num_selected_nodes; i++) {
      auto node = ImNodes::ObjectPoolFind(ImNodes::EditorContextGet().Nodes,
                                          num_selected_nodes[i]);
      if (node >= 0) {
        m_Nodes->SelectNode(num_selected_nodes[i]);
      }
    }
    delete num_selected_nodes;

    {
      int linkId;
      if (ImNodes::IsLinkDestroyed(&linkId))
        EraseLink(linkId);
    }

    // Handle creation of new links
    {
      int from, to;
      if (ImNodes::IsLinkCreated(&from, &to)) {

        do {
          nodes::Attribute *fromAttr = nullptr, *toAttr = nullptr;

          // Find the attributes that are connected by the link
          for (auto &node : m_Nodes->Nodes) {
            for (auto &attribute : node->GetAttributes()) {
              if (attribute.GetId() == from)
                fromAttr = &attribute;
              else if (attribute.GetId() == to)
                toAttr = &attribute;
            }
          }

          // If one of the attributes could not be found, the link is invalid
          // and can't be created
          if (fromAttr == nullptr || toAttr == nullptr)
            break;

          // If the attributes have different types, don't create the link
          if (fromAttr->GetType() != toAttr->GetType())
            break;

          // If the link tries to connect two input or two output attributes,
          // don't create the link
          if (fromAttr->GetIo() == toAttr->GetIo())
            break;

          // If the link tries to connect to a input attribute that already has
          // a link connected to it, don't create the link
          if (!toAttr->GetConnectedAttributes().empty())
            break;

          // Add a new link to the current workspace
          auto newLink = m_Links.emplace_back(from, to);

          // Add the link to the attributes that are connected by it
          fromAttr->AddConnectedAttribute(newLink.GetId(), toAttr);
          toAttr->AddConnectedAttribute(newLink.GetId(), fromAttr);

        } while (false);
      }
    }

    {
      const int selectedLinkCount = ImNodes::NumSelectedLinks();
      if (selectedLinkCount > 0 && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        std::vector<int> selectedLinks;
        selectedLinks.resize(static_cast<size_t>(selectedLinkCount));
        ImNodes::GetSelectedLinks(selectedLinks.data());
        ImNodes::ClearLinkSelection();

        for (const int id : selectedLinks)
          EraseLink(id);
      }
    }

    {
      const int selectedNodeCount = ImNodes::NumSelectedNodes();
      if (selectedNodeCount > 0 && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        std::vector<int> selectedNodes;
        selectedNodes.resize(static_cast<size_t>(selectedNodeCount));
        ImNodes::GetSelectedNodes(selectedNodes.data());
        ImNodes::ClearNodeSelection();

        EraseNodes(selectedNodes);
      }
    }

    // processing
    if (!m_thread.joinable()) {
      if (ImGui::Button(ICON_VS_DEBUG_START)) {
        ProcessNodes();
      }
    } else {
      if (ImGui::Button(ICON_VS_DEBUG_STOP)) {
        m_continuousProcessing = false;
      }
    }
    ImGui::SameLine();
    ImGui::Checkbox("Continuous Processing", &m_continuousProcessing);
  }

  void Editor::EraseLink(int id) {
    auto link = std::find_if(m_Links.begin(), m_Links.end(),
                             [&id](auto link) { return link.GetId() == id; });

    if (link == m_Links.end()) {
      return;
    }

    for (auto &node : m_Nodes->Nodes) {
      for (auto &attribute : node->GetAttributes())
        attribute.RemoveConnectedAttribute(id);
    }
    m_Links.erase(link);
  }

  void Editor::EraseNodes(std::vector<int> const &ids) {
    // Loop over the IDs of all nodes that should be removed
    for (int id : ids) {
      auto node =
          std::find_if(m_Nodes->Nodes.begin(), m_Nodes->Nodes.end(),
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
          std::find_if(m_Nodes->Nodes.begin(), m_Nodes->Nodes.end(),
                       [&id](auto const &node) { return node->GetId() == id; });

      std::erase_if(m_EndNodes,
                    [&id](auto const &node) { return node->GetId() == id; });

      m_Nodes->Nodes.erase(node);
    }
  }

  void Editor::ProcessNodes() {
    if (m_thread.joinable())
      m_thread.join();

    m_thread = std::thread([this]() {
      m_CurrNodeError = std::nullopt;

      do {
        for (auto &endNode : m_EndNodes) {
          endNode->ResetOutputValue();

          for (auto &node : m_Nodes->Nodes) {
            node->Reset();
            node->ResetProcessedInputs();
          }

          endNode->Process();
        }
      } while (m_continuousProcessing);

      m_thread.detach();
    });
  }

  void Editor::DrawNode(nodes::Node & node) {

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
      node.RenderErrors();
      ImNodes::EndNodeTitleBar();

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
          attribute.Render();
          ImNodes::EndInputAttribute();
        } else if (attribute.GetIo() == nodes::Attribute::IO::Out) {
          ImNodes::BeginOutputAttribute(attribute.GetId(),
                                        ImNodesPinShape(pin_shape + 1));
          attribute.Render();
          ImNodes::EndOutputAttribute();
        }
      }

      ImGui::PopStyleVar();
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
        m_Nodes->Nodes.push_back(node);
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
