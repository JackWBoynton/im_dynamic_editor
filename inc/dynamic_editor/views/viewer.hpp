#pragma once

namespace dynamic_editor::views {

class Viewer {
public:
  Viewer() = default;

  void RenderWindowed();
  void Render();
};

} // namespace dynamic_editor::views
