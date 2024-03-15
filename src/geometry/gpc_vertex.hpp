#pragma once

#include <iostream>
#include <sstream>

namespace gpc {

/* Polygon vertex structure          */
class gpc_vertex {
public:
  double x = 0.0; /* Vertex x component                */
  double y = 0.0; /* vertex y component                */

public:
  gpc_vertex() = default;
  gpc_vertex(double in_x, double in_y) : x(in_x), y(in_y) {}
  ~gpc_vertex() = default;

  friend std::istream &operator>>(std::istream &is, gpc_vertex &vertex) {
    is >> vertex.x >> vertex.y;
    return is;
  }

  friend std::ostream &operator<<(std::ostream &os, const gpc_vertex &vertex) {
    os << vertex.x << " " << vertex.y;
    return os;
  }

  std::string to_string() const {
    std::stringstream ss;
    ss << "gpc_vertex: (" << x << ", " << y << ")";
    return ss.str();
  }
};

} // namespace gpc