#include <dynamic_editor/nodes/attribute.hpp>
#include <string>
#include <utility>

namespace dynamic_editor::nodes {

int Attribute::s_id = 1;

Attribute::Attribute(IO io_type, Type type, std::string name)
    : m_Id(s_id++), m_Io(io_type), m_Type(type), m_Name(std::move(name)) {
  switch (m_Type) {
  case Type::Boolean:
    m_DefaultValue = false;
    break;
  case Type::Int:
    m_DefaultValue = 0;
    break;
  case Type::Float:
    m_DefaultValue = 0.0f;
    break;
  default:
    break;
  }
}

Attribute::~Attribute() {
  for (auto &[linkId, attr] : this->GetConnectedAttributes()) {
    attr->RemoveConnectedAttribute(linkId);
  }
}

void Attribute::SetIdCounter(int id) {
  if (id > s_id) {
    s_id = id;
  }
}

} // namespace dynamic_editor::nodes
