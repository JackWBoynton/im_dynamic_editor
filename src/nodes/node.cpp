
#include <dynamic_editor/nodes/attribute.hpp>
#include <dynamic_editor/nodes/node.hpp>
#include <dynamic_editor/utils/imgui_extras.hpp>

#include <atomic>
#include <cmath>
#include <cstddef>
#include <imnodes.h>
#include <string>
#include <utility>
#include <vector>

namespace dynamic_editor::nodes {

int Node::s_Id = 1;
static std::atomic<bool> s_interrupted;

void NodeHolder::SelectNode(const int id) {
  for (auto &node : Nodes) {
    if (node->GetId() == id) {
      SelectedNodes.insert(node);
      break;
    }
  }
}

void Node::Interrupt() { s_interrupted = true; }

void Node::MarkInputProcessed(size_t index) {
  auto const &[iter, inserted] = m_ProcessedInputs.insert(index);
  if (!inserted) {
    throw std::runtime_error("Node recursively processing input!");
  }

  if (s_interrupted) {
    s_interrupted = false;
    ThrowNodeError("Execution interrupted!");
  }
}

Node::Node(std::string title, std::vector<Attribute> attributes)
    : m_Id(s_Id++), m_Title(std::move(title)),
      m_Attributes(std::move(attributes)) {
  for (auto &attr : m_Attributes) {
    attr.SetParentNode(this);
  }
}

void Node::SetIdCounter(int id) {
  if (id > s_Id)
    s_Id = id;
}

void Node::WrapDrawNode() {
  try {
    DrawEditorNode();
    // clear draw error
    if (m_State & NodeState_DRAW_ERROR) {
      m_State &= ~NodeState_DRAW_ERROR;
    }
  } catch (const std::exception &e) {
    SetState(NodeState_DRAW_ERROR, "TODO:");
  }
}

auto Node::GetValueOnInput(size_t index) -> Attribute::ValueType & {
  auto *attribute = this->GetConnectedInputAttribute(index);
  auto &output_data = [&]() -> Attribute::ValueType & {
    if (attribute != nullptr) {
      MarkInputProcessed(index);
      attribute->GetParentNode()->Process();
      UnmarkInputProcessed(index);

      return attribute->GetOutputValue();
    }
    return this->GetAttribute(index).GetOutputValue();
  }();

  return output_data;
}

void Node::SetFloatOnOutput(size_t index, float value) {
  if (index >= this->GetAttributes().size()) {
    ThrowNodeError("Attribute index out of bounds!");
  }

  auto &attribute = this->GetAttributes()[index];

  if (attribute.GetIo() != Attribute::IO::Out) {
    ThrowNodeError("Tried to set output data of an input attribute!");
  }

  if (attribute.GetType() != Attribute::Type::Float) {
    ThrowNodeError("Tried to set float on non-float attribute!");
  }

  attribute.GetOutputValue() = value;
}

void Node::SetBoolOnOutput(size_t index, bool value) {
  if (index >= this->GetAttributes().size()) {
    ThrowNodeError("Attribute index out of bounds!");
  }

  auto &attribute = this->GetAttributes()[index];

  if (attribute.GetIo() != Attribute::IO::Out) {
    ThrowNodeError("Tried to set output data of an input attribute!");
  }

  if (attribute.GetType() != Attribute::Type::Boolean) {
    ThrowNodeError("Tried to set bool on non-bool attribute!");
  }

  attribute.GetOutputValue() = value;
}

} // namespace dynamic_editor::nodes
