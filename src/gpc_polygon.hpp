#pragma once

#include "enum.hpp"
#include "gpc_vertex_list.hpp"

namespace gpc {

class gpc_polygon /* Polygon set structure             */
{
public:
  // int num_contours = 0;               /* Number of contours in polygon     */
  std::vector<bool> hole;               /* Hole / external contour flags     */
  std::vector<gpc_vertex_list> contour; /* Contour array pointer */

public:
  gpc_polygon() = default;
  ~gpc_polygon() = default;

  inline int num_contours() const { return contour.size(); }

  friend std::istream &operator>>(std::istream &is, gpc_polygon &polygon);
  friend std::ostream &operator<<(std::ostream &os, const gpc_polygon &polygon);

  void add_contour(const gpc_vertex_list &in_contour, bool in_hole = false);
  std::vector<bbox> create_contour_bboxes() const;

  std::string to_string() const;
};

void minimax_test_diff(gpc_polygon *subj, gpc_polygon *clip);
void minimax_test_int(gpc_polygon *subj, gpc_polygon *clip);
void minimax_test(gpc_polygon *subj, gpc_polygon *clip, gpc_op op);

} // namespace gpc