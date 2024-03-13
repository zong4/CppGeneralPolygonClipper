#pragma once

#include <vector>

#include "bbox.hpp"
#include "gpc_vertex.hpp"

namespace gpc {

class gpc_vertex_list /* Vertex list structure             */
{
public:
  bool is_contributing = true;
  // int num_vertices = 0;           /* Number of vertices in list        */
  std::vector<gpc_vertex> vertex; /* Vertex array pointer */

public:
  gpc_vertex_list() = default;
  gpc_vertex_list(const std::vector<gpc_vertex> &in_vertex)
      : vertex(in_vertex) {}
  ~gpc_vertex_list() = default;

  inline int num_vertices() const { return vertex.size(); }

  friend std::istream &operator>>(std::istream &is,
                                  gpc_vertex_list &vertex_list);
  friend std::ostream &operator<<(std::ostream &os,
                                  const gpc_vertex_list &vertex_list);

  bbox create_bbox() const;

  std::string to_string() const;
};

} // namespace gpc