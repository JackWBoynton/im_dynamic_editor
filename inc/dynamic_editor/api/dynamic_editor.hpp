#pragma once

#include <dynamic_editor/views/editor.hpp>
#include <dynamic_editor/views/viewer.hpp>

#include <dynamic_editor/nodes/node.hpp>

#include "imgui.h"
#include "imgui_internal.h"

#include <set>

namespace dynamic_editor::api {

namespace impl {
void RegisterNodeType(nodes::NodeFactory const &factory);
}

void RegisterInitializer(const std::function<void()> &initializer);
void RunInitializers();

const std::vector<nodes::NodeFactory> &GetNodeFactories();

template <std::derived_from<nodes::Node> T, typename... Args>
void RegisterNodeType(std::string const &cat, std::string const &name,
                      std::string const &description, Args &&...args) {
  impl::RegisterNodeType(nodes::NodeFactory{
      cat, name, description,
      [=, ... args = std::forward<Args>(args)]() mutable {
        auto node = std::make_shared<T>(name, std::forward<Args>(args)...);
        node->SetName(name);
        node->SetTitle(name);
        return node;
      }});
}

class DynamicEditor {
public:
  DynamicEditor()
      : m_nodes(std::make_shared<std::vector<std::shared_ptr<nodes::Node>>>()),
        m_editor(m_nodes), m_viewer(m_nodes) {}

  void Render();
  void RenderWindowed();

private:
  void ConfigureDockspace();

  ImGuiID m_dockspace_id;
  ImGuiWindowClass m_dockspace_wc;
  ImGuiWindowClass m_dockspace_sub_wc;

  nodes::NodeHolder m_nodes{};
  views::Editor m_editor;
  views::Viewer m_viewer;
};

} // namespace dynamic_editor::api
