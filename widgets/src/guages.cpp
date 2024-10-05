
#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_internal.h"

#include <implot.h>

#include "codicons.hpp"

#include <dynamic_editor/utils/imgui_extras.hpp>

#include "guages.hpp"

using namespace ImGui;

namespace widgets {

namespace guages {

void GuageColorMap::Render(GuageColorMap &colorMap) {
  std::set<float> delete_keys;
  std::vector<std::pair<float, ImU32>>
      updated_entries; // Collect updated entries
  int i = 0;

  for (auto it = colorMap.Map.begin(); it != colorMap.Map.end(); ++it) {
    float key = it->first; // Local copy of the key
    auto &color = it->second;
    ImGui::PushID(i++);

    if (ImGui::Button(ICON_VS_REMOVE)) {
      delete_keys.insert(key);
      ImGui::PopID();
      continue;
    }

    ImGui::SameLine();
    ImGui::PushItemWidth(100);

    auto color_vec4 = ImColor(color).Value;
    if (ImGui::ColorPicker3("Color", &color_vec4.x)) {
      color = ImColor(color_vec4);
    }

    ImGui::SameLine();

    float new_key = key;
    if (ImGui::InputFloat("Value", &new_key)) {
      if (new_key != key) {
        delete_keys.insert(key); // Mark old key for deletion
        updated_entries.emplace_back(new_key, color);
      }
    }

    ImGui::PopItemWidth();
    ImGui::PopID();
  }

  ImGui::Separator();

  if (ImGui::Button(ICON_VS_ADD)) {
    colorMap.Map[99999] = IM_COL32(0, 0, 0, 0);
  }

  for (const auto &key : delete_keys) {
    colorMap.Map.erase(key);
  }

  for (const auto &entry : updated_entries) {
    colorMap.Map[entry.first] = entry.second;
  }
}

bool SimpleGuage(const char *label, float value, float min, float max,
                 GuageColorMap &colorMap, const char *format, float radius,
                 float thickness, float start_angle, float end_angle,
                 float threshold_indicator_div) {
  ImGuiWindow *window = GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ImGuiContext &g = *GImGui;
  const ImGuiStyle &style = g.Style;
  const ImGuiID id = window->GetID(label);
  ImVec2 label_size = CalcTextSize(label, nullptr, true);

  const ImVec2 pos = window->DC.CursorPos;

  // bool needs_wrap = label_size.x > radius * 2 - style.FramePadding.x * 2;

  // if (needs_wrap) {
  //   label_size.y += (int)(label_size.x / (radius * 2)) * CalcTextSize("A").y;
  // }

  const ImRect total_bb(pos,
                        pos + ImVec2(radius * 2, radius * 2) +
                            ImVec2(0, label_size.y + style.FramePadding.y) +
                            style.FramePadding * 2);
  ItemSize(total_bb, style.FramePadding.y);
  if (!ItemAdd(total_bb, id)) {
    return false;
  }

  if (min == max) {
    return false;
  }

  bool hovered, held;
  bool pressed = ButtonBehavior(total_bb, id, &hovered, &held,
                                ImGuiButtonFlags_MouseButtonRight);
  if (pressed) {
    MarkItemEdited(id);
  }

  auto label_pos =
      total_bb.GetTL() +
      ImVec2((total_bb.GetWidth() - label_size.x) / 2, style.FramePadding.y);

  // add the label above the guage in the center
  window->DrawList->AddText(label_pos, ImColor(style.Colors[ImGuiCol_Text]),
                            label);
  const auto center =
      total_bb.GetCenter() + ImVec2(0, label_size.y + style.ItemInnerSpacing.y);

  // lambda to convert from value to angle ( corrected for the start of 0.75 and
  // end of 2.25 )
  auto lerper = [&](float val) {
    return start_angle * IM_PI +
           (val - min) / (max - min) * (end_angle - start_angle) * IM_PI;
  };

  float angle_min = lerper(min);
  float angle_max = lerper(max);
  if (angle_min > angle_max) {
    std::swap(angle_min, angle_max);
  }
  if (angle_min == angle_max) {
    angle_max += 2 * IM_PI;
  }
  if (angle_max - angle_min < IM_PI / 6) {
    return false;
  }

  // validate that our color map has the same max

  auto color_ring_thickness = thickness / threshold_indicator_div;
  auto color_ring_radius = radius;

  // add the colors
  float current_angle = start_angle * IM_PI;
  for (const auto &[key, color] : colorMap.Map) {
    float next_angle = lerper(key);
    if (next_angle > angle_max) {
      continue;
    }
    if (current_angle < angle_min) {
      continue;
    }
    if (next_angle == current_angle) {
      continue;
    }
    window->DrawList->PathClear();
    window->DrawList->PathArcTo(center, color_ring_radius, current_angle,
                                next_angle, 32);
    window->DrawList->PathStroke(color, ImDrawFlags_None, color_ring_thickness);
    current_angle = next_angle;
  }

  // add the background of the guage
  auto value_ring_radius =
      radius - color_ring_thickness * 3.0f - radius / (16.0f);
  window->DrawList->PathClear();
  window->DrawList->PathArcTo(center, value_ring_radius, start_angle * IM_PI,
                              end_angle * IM_PI);
  window->DrawList->PathStroke(ImColor(style.Colors[ImGuiCol_MenuBarBg]),
                               ImDrawFlags_None, thickness);

  // add the current values bar of the correct color and angle
  float current_angle_value =
      std::max(start_angle * IM_PI, std::min(end_angle * IM_PI, lerper(value)));
  if (std::abs(current_angle_value - start_angle * IM_PI) < 0.01f) {
    return false;
  }
  window->DrawList->PathClear();
  window->DrawList->PathArcTo(center, value_ring_radius, start_angle * IM_PI,
                              current_angle_value);
  ImU32 value_color = 0;
  ImU32 last_color = 0;
  for (const auto &[key, color] : colorMap.Map) {
    if (value <= key) {
      value_color = color;
      break;
    }
    last_color = color;
  }
  if (value_color == 0) {
    value_color = last_color;
  }
  window->DrawList->PathStroke(value_color, ImDrawFlags_None, thickness);

  // add a white line at the end of the value to make it look like a needle
  auto value_ring_thickness = thickness + 1.0f;
  window->DrawList->PathClear();
  window->DrawList->PathArcTo(center, value_ring_radius,
                              current_angle_value - 0.01f * IM_PI / 2,
                              current_angle_value + 0.01f * IM_PI / 2);
  window->DrawList->PathStroke(ImColor(ImVec4(1, 1, 1, 1)), ImDrawFlags_None,
                               value_ring_thickness);

  // add the value in the middle of the guage
  char buffer[64];
  snprintf(buffer, sizeof(buffer), format, value);
  const ImVec2 value_size = ImGui::CalcTextSize(buffer, nullptr, true);
  window->DrawList->AddText(center - value_size / 2, value_color, buffer);

  return true;
}
} // namespace guages

} // namespace widgets
