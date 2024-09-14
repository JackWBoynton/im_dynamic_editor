
#include <dynamic_editor/api/dynamic_editor.hpp>

#include <guages.hpp>

void RegisterWidgets() {
  dynamic_editor::api::RegisterNodeType<SimpleGuageNode>(
      "Visualizations", "Simple Guage", "A simple guage widget");
}

struct AutoRegisterWidgets {
  AutoRegisterWidgets() { RegisterWidgets(); }
};

static volatile AutoRegisterWidgets s_auto_register_widgets;

// This function will ensure the static object is referenced
extern "C" void EnsureAutoRegisterWidgets() { (void)s_auto_register_widgets; }
