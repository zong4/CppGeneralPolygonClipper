#pragma once

#include <algorithm>

#include "../utilis/gpc_enum.hpp"
#include "gpc_vertex_list.hpp"

namespace gpc {

// Polygon set structure
class gpc_polygon {
public:
  std::vector<bool> hole;               // Hole / external contour flags
  std::vector<gpc_vertex_list> contour; // Contour array pointer

public:
  gpc_polygon() = default;
  gpc_polygon(const std::vector<gpc_vertex_list> &in_contour,
              const std::vector<bool> &in_hole)
      : contour(in_contour), hole(in_hole) {}
  gpc_polygon(const std::vector<std::vector<gpc_vertex>> &in_contour,
              const std::vector<bool> &in_hole);
  ~gpc_polygon() = default;

  inline int num_contours() const { return contour.size(); }

  bool operator==(const gpc_polygon &rhs) const;

  friend std::istream &operator>>(std::istream &is, gpc_polygon &polygon);
  friend std::ostream &operator<<(std::ostream &os, const gpc_polygon &polygon);

  void add_contour(const gpc_vertex_list &in_contour, bool in_hole = false);
  std::vector<gpc_bbox> create_contour_bboxes() const;

  std::string to_string() const;
};

void minimax_test_diff(const gpc_polygon &subj, gpc_polygon &clip);
void minimax_test_int(gpc_polygon &subj, gpc_polygon &clip);
void minimax_test(gpc_polygon &subj, gpc_polygon &clip, gpc_op op);

bool equal_sort(const gpc_polygon &subj, const gpc_polygon &clip);

} // namespace gpc