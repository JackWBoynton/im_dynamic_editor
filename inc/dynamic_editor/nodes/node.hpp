
#pragma once

#include <dynamic_editor/nodes/attribute.hpp>
#include <dynamic_editor/utils/imgui_extras.hpp>

#include "codicons.hpp"

#include <cstddef>
#include <functional>
#include <list>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "imgrid.h"
#include "imgui.h"
#include <nlohmann/json.hpp>

namespace dynamic_editor::nodes {

class Node;
using NodeFactoryFunc = std::function<std::shared_ptr<Node>()>;

struct NodeHolder {
  std::vector<std::shared_ptr<Node>> Nodes;
  std::set<std::shared_ptr<Node>> SelectedNodes;

  void SelectNode(const int id);
  void ResetSelectedNodes() { SelectedNodes.clear(); }
};

struct NodeFactory {
  std::string Cat;
  std::string Name;
  std::string Description;
  NodeFactoryFunc Func;

  bool operator==(NodeFactory const &rhs) const {
    return Cat == rhs.Cat && Name == rhs.Name;
  }
};
inline bool operator<(NodeFactory const &lhs, NodeFactory const &rhs) {
  return lhs.Cat < rhs.Cat || (lhs.Cat == rhs.Cat && lhs.Name < rhs.Name);
}

typedef int NodeState;

enum NodeState_ {
  NodeState_OK = 1 << 0,
  NodeState_OVERRUN = 1 << 1,
  NodeState_NODE_LOGIC_ERROR = 1 << 2,
  NodeState_DRAW_ERROR = 1 << 3,
};

class Node {
public:
  Node(std::string title, std::vector<Attribute> attributes);
  virtual ~Node() = default;

  [[nodiscard]] auto GetId() const -> int { return m_Id; }
  void SetId(int id) { m_Id = id; }

  void SetPosition(ImVec2 pos) { m_Position = pos; }
  [[nodiscard]] auto GetPosition() const -> ImVec2 { return m_Position; }

  void WrapDrawNode();

  virtual void DrawCommonProperties() {
    ImGui::InputText("Title", m_Title);
    ImGui::Checkbox("Render Viewer Node", &m_ShouldRenderViewer);
  }

  void DrawProperties() {
    ImGui::BeginSubWindow(GetTitle().c_str());
    DrawCommonProperties();
    ImGui::SeparatorText("Properties");
    DrawPropertiesContent();
    ImGui::EndSubWindow();
  }

  bool &ShouldRenderViewer() { return m_ShouldRenderViewer; }

  virtual void DrawPropertiesContent() {}

  virtual void DrawEditorNode() {}
  virtual void DrawViewerNodeContent() = 0;

  virtual void CheckForErrors() {}
  virtual void RenderErrors() {
    if (GetHasError()) {
      ImGui::SameLine();
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
      ImGui::Text(ICON_VS_ERROR);
      ImGui::PopStyleColor();
      if (ImGui::BeginItemTooltip()) {
        ImGui::Text("%s", GetError().c_str());
        ImGui::EndTooltip();
      }
    }
    if (GetHasWarning()) {
      ImGui::SameLine();
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1));
      ImGui::Text(ICON_VS_WARNING);
      ImGui::PopStyleColor();
      if (ImGui::BeginItemTooltip()) {
        ImGui::Text("%s", GetWarning().c_str());
        ImGui::EndTooltip();
      }
    }
  }

  // only renders content if there is no error
  virtual void Process() = 0;
  virtual void Dump(nlohmann::json &data) const {}
  virtual void Load(nlohmann::json const &data) {}

  [[nodiscard]] auto GetTitle() const -> std::string { return m_Title; }
  void SetTitle(std::string const &title) { m_Title = title; }
  [[nodiscard]] auto GetName() const -> std::string { return m_Name; }
  void SetName(std::string const &name) { m_Name = name; }

  [[nodiscard]] auto GetAttributes() -> std::vector<Attribute> & {
    return m_Attributes;
  }
  [[nodiscard]] auto GetAttributes() const -> std::vector<Attribute> const & {
    return m_Attributes;
  }

  auto GetValueOnInput(size_t index) -> Attribute::ValueType &;
  template <typename T> auto GetTOnInput(size_t index, T *value) -> bool {
    auto const &v = this->GetValueOnInput(index);
    if (auto *p = std::get_if<T>(&v)) {
      *value = *p;
      return true;
    }

    return false;
  }
  template <typename T> auto GetTOnInput(size_t index) -> T {
    auto const &v = this->GetValueOnInput(index);
    if (auto *p = std::get_if<T>(&v)) {
      return *p;
    }
    return T{};
  }

  template <typename T> T *GetTPtrOnInput(size_t index) {
    auto &v = this->GetValueOnInput(index);
    if (auto *p = std::get_if<T>(&v)) {
      return p;
    }
    return nullptr;
  }

  void SetFloatOnOutput(size_t index, float value);
  void SetBoolOnOutput(size_t index, bool value);

  void ResetOutputValue() {
    for (auto &attribute : m_Attributes) {
      attribute.ResetOutputValue();
    }
  }

  void ResetProcessedInputs() { m_ProcessedInputs.clear(); }

  static void SetIdCounter(int id);

  [[nodiscard]] auto GetState() const -> NodeState { return m_State; }
  void SetState(NodeState state, std::string message) {
    m_State |= state;
    m_CurrentError = std::move(message);
  }
  void ClearState(NodeState state) {
    m_State &= ~state;
    m_CurrentError.clear();
  }
  [[nodiscard]] auto GetCurrentError() const -> std::string {
    return m_CurrentError;
  }

  [[nodiscard]] auto GetStateful() const -> bool { return m_Stateful; }
  void ResetStatefulState() { m_ShouldUpdate = false; }
  void SetStatefulState() { m_ShouldUpdate = true; }
  [[nodiscard]] auto GetShouldUpdate() const -> bool { return m_ShouldUpdate; }

  bool GetHasError() const { return !m_Error.empty(); }
  std::string GetError() const { return m_Error; }
  void SetError(std::string const &error) { m_Error = error; }
  void ClearError() { m_Error.clear(); }

  bool GetHasWarning() const { return !m_Warning.empty(); }
  std::string GetWarning() const { return m_Warning; }
  void SetWarning(std::string const &warning) { m_Warning = warning; }
  void ClearWarning() { m_Warning.clear(); }

  struct NodeError {
    Node *NodePtr;
    std::string Message;
  };

  static void Interrupt();

protected:
  int m_Id;
  std::string m_Title;
  std::string m_Name;
  ImVec2 m_Position;
  std::vector<Attribute> m_Attributes;
  bool m_Stateful;
  std::string m_CurrentError;
  bool m_ShouldUpdate{true};
  std::set<size_t> m_ProcessedInputs;
  std::string m_Error;
  std::string m_Warning;
  bool m_ShouldRenderViewer{true};

  static int s_Id;

protected:
  auto GetAttribute(size_t index) -> Attribute & {
    if (index >= this->GetAttributes().size()) {
      throw std::runtime_error("Attribute index out of bounds!");
    }

    return this->GetAttributes()[index];
  }

  auto GetConnectedInputAttribute(size_t index) -> Attribute * {
    auto const &connected_attribute =
        this->GetAttribute(index).GetConnectedAttributes();

    if (connected_attribute.empty()) {
      return nullptr;
    }

    return connected_attribute.begin()->second;
  }

  void MarkInputProcessed(size_t index);
  void UnmarkInputProcessed(size_t index) { m_ProcessedInputs.erase(index); }

  NodeState m_State{NodeState_OK};

  [[noreturn]] void ThrowNodeError(std::string const &message) {
    throw NodeError{this, message};
  }

  void SetAttributes(std::vector<Attribute> attributes) {
    m_Attributes = std::move(attributes);

    for (auto &attr : m_Attributes) {
      attr.SetParentNode(this);
    }
  }
};

} // namespace dynamic_editor::nodes
