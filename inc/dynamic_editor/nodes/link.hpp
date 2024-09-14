#pragma once

namespace dynamic_editor::nodes {

class Link {
public:
  Link(int from, int to);

  [[nodiscard]] auto GetId() const -> int { return m_Id; }
  void SetId(int id) { m_Id = id; }

  [[nodiscard]] auto GetFromId() const -> int { return m_From; }
  [[nodiscard]] auto GetToId() const -> int { return m_To; }

  static void SetIdCounter(int id);

private:
  int m_Id;
  int m_From, m_To;

  static int s_id;
};

} // namespace dynamic_editor::nodes
