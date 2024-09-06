#pragma once

namespace dynamic_editor::views {

class Editor {
public:
  Editor() = default;

  void RenderWindowed();
  void Render();

private:
  void DrawContextMenus();
  void DrawNode();
};

} // namespace dynamic_editor::views
