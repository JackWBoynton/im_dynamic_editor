#include <dynamic_editor/nodes/link.hpp>

namespace dynamic_editor::nodes {

int Link::s_id = 1;

Link::Link(int from, int to) : m_Id(s_id++), m_From(from), m_To(to) {}

void Link::SetIdCounter(int id) {
  if (id > s_id) {
    s_id = id;
  }
}

} // namespace dynamic_editor::nodes
