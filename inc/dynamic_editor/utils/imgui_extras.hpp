#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <string>

namespace ImGui {

void BeginSubWindow(char const *label, ImVec2 size = ImVec2(0, 0),
                    ImGuiChildFlags flags = ImGuiChildFlags_None);

void EndSubWindow();

void TextUnformattedCentered(char const *text);

bool InputText(char const *label, std::u8string &buffer,
               ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
bool InputText(char const *label, std::string &buffer,
               ImGuiInputTextFlags flags = ImGuiInputTextFlags_None);
} // namespace ImGui
