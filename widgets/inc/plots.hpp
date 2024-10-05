#pragma once

#include <dynamic_editor/nodes/node.hpp>

#include "imgui.h"
#include "imgui_internal.h"

#include <map>

using namespace dynamic_editor::nodes;

namespace widgets {

namespace guages {

bool SimpleGuage(const char *label, float value, float min, float max,
                 GuageColorMap &colorMap, const char *format = "%.2f",
                 float radius = 50, float thickness = 10,
                 float start_angle = 0.75f, float end_angle = 2.25f,
                 float threshold_indicator_div = 6.0f);

} // namespace guages

} // namespace widgets

class TimeseriesPlotNode : public Node {
public:
  TimeseriesPlotNode(std::string name)
      : Node(name, {
                       {Attribute::IO::In, Attribute::Type::Float, "Value"},
                   }) {
    memcpy(label, name.c_str(), std::min(static_cast<int>(name.size()), 256));
  }
  void DrawEditorNode() override {
    Node::DrawEditorNode(); // adds title bar
    ImGui::InputText("Label", label, 256);
    ImGui::InputFloat("Min Value", &min);
  }
  void DrawViewerNodeContent() override {
    widgets::guages::SimpleGuage(label, value, min, max, colorMap, format,
                                 radius, thickness, start_angle, end_angle,
                                 threshold_indicator_div);
  }
  void Process() override {}

private:
  char label[256];
  float value{50};
  float min{0};
  float max{100};
  widgets::guages::GuageColorMap colorMap;
  const char *format = "%.2f";
  float radius{200};
  float thickness{12};
  float start_angle{0.75f};
  float end_angle{2.25f};
  float threshold_indicator_div{6.0f};
};
