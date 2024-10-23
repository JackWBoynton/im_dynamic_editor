#define IMGUI_DEFINE_MATH_OPERATORS

#include <dynamic_editor/utils/imgui_extras.hpp>

#include "imgui.h"
#include "imgui_internal.h"

#include "implot/implot.h"
#include <implot/implot_internal.h>

#include <fmt/format.h>

using namespace ImGui;

namespace ImGuiExtras {

int UpdateStringSizeCallback(ImGuiInputTextCallbackData *data) {
  if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
    auto &string = *static_cast<std::string *>(data->UserData);

    string.resize(data->BufTextLen);
    data->Buf = string.data();
  }

  return 0;
}

void BeginSubWindow(char const *label, ImVec2 size, ImGuiChildFlags flags) {
  bool const has_menu_bar = !std::string_view(label).empty();

  PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0F);
  if (BeginChild(fmt::format("{}##SubWindow", label).c_str(), size,
                 ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY | flags,
                 has_menu_bar ? ImGuiWindowFlags_MenuBar
                              : ImGuiWindowFlags_None)) {
    if (has_menu_bar && BeginMenuBar()) {
      TextUnformatted(label);
      EndMenuBar();
    }
  }
  PopStyleVar();
}

void EndSubWindow() { EndChild(); }

void TextUnformattedCentered(char const *text) {
  auto available_space = ImGui::GetContentRegionAvail();

  std::string draw_string;
  auto const *text_end = text + strlen(text);
  for (auto const *wrap_pos = text; wrap_pos != text_end;) {
    wrap_pos = ImGui::GetFont()->CalcWordWrapPositionA(
        1, wrap_pos, text_end, available_space.x * 0.8F);
    draw_string += std::string(text, wrap_pos) + "\n";
    text = wrap_pos;
  }

  if (!draw_string.empty())
    draw_string.pop_back();

  auto text_size = ImGui::CalcTextSize(draw_string.c_str());

  ImPlot::AddTextCentered(ImGui::GetWindowDrawList(),
                          ImGui::GetCursorScreenPos() + ImVec2(available_space.x / 2, available_space.y / 2) -
                              ImVec2(0, text_size.y / 2),
                          ImGui::GetColorU32(ImGuiCol_Text),
                          draw_string.c_str());
}

bool InputText(char const *label, std::u8string &buffer,
               ImGuiInputTextFlags flags) {
  return ImGui::InputText(label, reinterpret_cast<char *>(buffer.data()),
                          buffer.size() + 1,
                          ImGuiInputTextFlags_CallbackResize | flags,
                          UpdateStringSizeCallback, &buffer);
}
bool InputText(char const *label, std::string &buffer,
               ImGuiInputTextFlags flags) {
  return ImGui::InputText(label, buffer.data(), buffer.size() + 1,
                          ImGuiInputTextFlags_CallbackResize | flags,
                          UpdateStringSizeCallback, &buffer);
}

} // namespace ImGuiExtras
