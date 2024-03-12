#pragma once

#include "gpc_vertex_list.hpp"

namespace gpc {

class gpc_polygon /* Polygon set structure             */
{
public:
  // int num_contours = 0;               /* Number of contours in polygon     */
  std::vector<bool> hole;               /* Hole / external contour flags     */
  std::vector<gpc_vertex_list> contour; /* Contour array pointer */

  inline int num_contours() const { return contour.size(); }

public:
  gpc_polygon() = default;
  ~gpc_polygon() = default;

  friend std::istream &operator>>(std::istream &is, gpc_polygon &polygon);
  friend std::ostream &operator<<(std::ostream &os, const gpc_polygon &polygon);

  void add_contour(const gpc_vertex_list &in_contour, bool in_hole = false);

  std::string to_string() const;
};

} // namespace gpc