#pragma once

#include <dynamic_editor/nodes/node.hpp>

#include "imgui.h"
#include "imgui_internal.h"

#include <map>

using namespace dynamic_editor::nodes;

namespace widgets {

namespace guages {

struct GuageColorMap {
  std::map<float, ImU32> Map;

  static bool Render();

  GuageColorMap(std::initializer_list<std::pair<const float, ImU32>> init)
      : Map(init) {}
  GuageColorMap() = default;

  static GuageColorMap *Editing;
};

bool SimpleGuage(const char *label, float value, float min, float max,
                 GuageColorMap &colorMap, const char *format = "%.2f",
                 float radius = 50, float thickness = 10,
                 float start_angle = 0.75f, float end_angle = 2.25f,
                 float threshold_indicator_div = 6.0f);

} // namespace guages

} // namespace widgets

class SimpleGuageNode : public Node {
public:
  SimpleGuageNode(std::string name)
      : Node(name, {
                       {Attribute::IO::In, Attribute::Type::Float, "Value"},
                       {Attribute::IO::In, Attribute::Type::Float, "Min Value"},
                       {Attribute::IO::In, Attribute::Type::Float, "Max Value"},
                   }) {
    colorMap = widgets::guages::GuageColorMap{{50, IM_COL32(0, 153, 0, 255)},
                                              {75, IM_COL32(255, 255, 0, 255)},
                                              {100, IM_COL32(255, 0, 0, 255)}};
    memcpy(label, name.c_str(), std::min(static_cast<int>(name.size()), 256));
  }
  void CheckForErrors() override {
    if (GetTOnInput<float>(1) == GetTOnInput<float>(2)) {
      SetError("Min and Max cannot be the same");
    }
    if (GetTOnInput<float>(1) > GetTOnInput<float>(2)) {
      SetError("Min cannot be greater than Max");
    }

    if (GetTOnInput<float>(0) < GetTOnInput<float>(1)) {
      SetWarning("Value is less than Min");
    }
    if (GetTOnInput<float>(0) > GetTOnInput<float>(2)) {
      SetWarning("Value is greater than Max");
    }
  }
  void DrawEditorNode() override {
    Node::DrawEditorNode(); // adds title bar
    ImGui::SetNextItemWidth(50);
    ImGui::InputText("Label", label, 256);
  }

  void DrawViewerNodeContent() override {
    widgets::guages::SimpleGuage(
        label, GetTOnInput<float>(0), GetTOnInput<float>(1),
        GetTOnInput<float>(2), colorMap, format, radius, thickness, start_angle,
        end_angle, threshold_indicator_div);
  }
  void Process() override {}

private:
  char label[256];
  widgets::guages::GuageColorMap colorMap;
  const char *format = "%.2f";
  float radius{200};
  float thickness{12};
  float start_angle{0.75f};
  float end_angle{2.25f};
  float threshold_indicator_div{6.0f};
};
