#pragma once

#include <map>
#include <string>
#include <variant>

#include "imgui.h"

namespace dynamic_editor::nodes {
class Node;

class Attribute {
public:
  using ValueType = std::variant<std::monostate, float, bool, int>;
  enum class Type { Float, Boolean, Int, Buffer };

  enum class IO { In, Out };
  Attribute(IO io_type, Type type, std::string name);
  ~Attribute();

  [[nodiscard]] auto GetId() const -> int { return m_Id; }
  void SetId(int id) { m_Id = id; }

  [[nodiscard]] auto GetIo() const -> IO { return m_Io; }
  [[nodiscard]] auto GetType() const -> Type { return m_Type; }
  [[nodiscard]] auto GetName() const -> std::string { return m_Name; }

  void AddConnectedAttribute(int link_id, Attribute *to) {
    m_ConnectedAttributes.insert({link_id, to});
  }
  void RemoveConnectedAttribute(int link_id) {
    m_ConnectedAttributes.erase(link_id);
  }
  [[nodiscard]] auto GetConnectedAttributes() -> std::map<int, Attribute *> & {
    return m_ConnectedAttributes;
  }

  [[nodiscard]] auto GetParentNode() const -> Node * { return m_ParentNode; }

  static void SetIdCounter(int id);

  auto GetOutputValue() -> ValueType & {
    if (!std::holds_alternative<std::monostate>(m_OutputValue))
      return m_OutputValue;
    return m_DefaultValue;
  }

  void ResetOutputValue() { m_OutputValue = std::monostate{}; }

  void Render() {
    std::visit(
        [this](auto &value) {
          ImGui::PushItemWidth(100);

          using T = std::decay_t<decltype(value)>;
          if constexpr (std::is_same_v<T, float>) {
            ImGui::InputScalar(GetName().c_str(), ImGuiDataType_Float, &value);
          } else if constexpr (std::is_same_v<T, int>) {
            ImGui::InputScalar(GetName().c_str(), ImGuiDataType_S64, &value);
          } else if constexpr (std::is_same_v<T, bool>) {
            ImGui::Checkbox(GetName().c_str(), &value);
          } else {
            ImGui::Text("%s", GetName().c_str());
          }
        },
        m_DefaultValue);
  }

private:
  int m_Id;
  IO m_Io;
  Type m_Type;
  std::string m_Name;
  std::map<int, Attribute *> m_ConnectedAttributes;

  Node *m_ParentNode = nullptr;

  ValueType m_DefaultValue;
  ValueType m_OutputValue;

  friend class Node;
  void SetParentNode(Node *node) { m_ParentNode = node; }

  static int s_id;
};
} // namespace dynamic_editor::nodes
