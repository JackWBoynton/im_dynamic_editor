#pragma once

#include <dynamic_editor/nodes/node.hpp>

#include "imgui.h"
#include "imgui_internal.h"

#include <map>

using namespace dynamic_editor::nodes;

class FloatSliderNode : public Node {
public:
  FloatSliderNode(std::string name)
      : Node(name, {
                       {Attribute::IO::In, Attribute::Type::Float, "Min"},
                       {Attribute::IO::In, Attribute::Type::Float, "Max"},
                       {Attribute::IO::Out, Attribute::Type::Float, "Value"},
                   }) {}
  void DrawViewerNodeContent() override {
    ImGui::SetNextItemWidth(100.0f);
    if (m_IsVertical)
      ImGui::VSliderFloat(GetTitle().c_str(), ImVec2(18, 160),
                          GetTPtrOnInput<float>(2), GetTOnInput<float>(0),
                          GetTOnInput<float>(1));
    else
      ImGui::SliderFloat(GetTitle().c_str(), GetTPtrOnInput<float>(2),
                         GetTOnInput<float>(0), GetTOnInput<float>(1));
  }

  void CheckForErrors() override {
    if (GetTOnInput<float>(0) == GetTOnInput<float>(1)) {
      SetError("Min and Max cannot be the same");
    }
    if (GetTOnInput<float>(0) > GetTOnInput<float>(1)) {
      SetError("Min cannot be greater than Max");
    }

    if (GetTOnInput<float>(2) < GetTOnInput<float>(0)) {
      SetWarning("Value is less than Min");
    }
    if (GetTOnInput<float>(2) > GetTOnInput<float>(1)) {
      SetWarning("Value is greater than Max");
    }
  }

  void DrawPropertiesContent() override {
    ImGui::InputFloat("Min", GetTPtrOnInput<float>(0));
    ImGui::InputFloat("Max", GetTPtrOnInput<float>(1));
    ImGui::Text("Value: %f", GetTOnInput<float>(2));
    ImGui::Checkbox("Vertical", &m_IsVertical);
  }

  void Process() override { SetFloatOnOutput(2, GetTOnInput<float>(2)); }

private:
  bool m_IsVertical = false;
};
