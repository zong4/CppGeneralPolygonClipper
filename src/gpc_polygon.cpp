#include "gpc_polygon.hpp"

std::istream &gpc::operator>>(std::istream &is, gpc::gpc_polygon &polygon) {
  int num_contours;
  is >> num_contours;

  for (int i = 0; i < num_contours; ++i) {
    bool hole;
    is >> hole;

    gpc_vertex_list vertex_list;
    is >> vertex_list;

    if (vertex_list.num_vertices() == 0) {
      continue;
    }

    polygon.add_contour(vertex_list, hole);
  }

  return is;
}

std::ostream &gpc::operator<<(std::ostream &os,
                              const gpc::gpc_polygon &polygon) {
  os << polygon.num_contours() << "\n";

  for (int i = 0; i < polygon.num_contours(); ++i) {
    os << polygon.hole[i] << "\n";
    os << polygon.contour[i];
  }

  // os << polygon.to_string();

  return os;
}

void gpc::gpc_polygon::add_contour(const gpc_vertex_list &in_contour,
                                   bool in_hole) {
  contour.push_back(in_contour);
  hole.push_back(in_hole);
}

std::string gpc::gpc_polygon::to_string() const {
  std::stringstream ss;

  ss << "gpc_polygon: " << num_contours() << " contours\n";

  for (int i = 0; i < num_contours(); ++i) {
    ss << "contour " << i << ":\n";
    ss << "hole: " << hole[i] << "\n";

    ss << contour[i].to_string();
  }

  return ss.str();
}

std::vector<gpc::bbox> gpc::gpc_polygon::create_contour_bboxes() const {
  std::vector<bbox> bboxes;

  for (int i = 0; i < num_contours(); ++i) {
    bboxes.push_back(contour[i].create_bbox());
  }

  return bboxes;
}

void gpc::minimax_test_diff(gpc::gpc_polygon *subj, gpc::gpc_polygon *clip) {
  std::vector<bbox> s_bboxs = subj->create_contour_bboxes();
  std::vector<bbox> c_bboxs = clip->create_contour_bboxes();

  // 主的轮廓都有用，只需要检查辅的轮廓
  for (int c = 0; c < clip->num_contours(); ++c) {
    bool overlap = false;

    for (int s = 0; s < subj->num_contours(); ++s) {
      if (is_intersect(s_bboxs[s], c_bboxs[c])) {
        overlap = true;
      }
    }

    if (!overlap) {
      /* Flag non contributing status by negating vertex count */
      clip->contour[c].is_contributing = false;
    }
  }
}

void gpc::minimax_test_int(gpc::gpc_polygon *subj, gpc::gpc_polygon *clip) {
  std::vector<bbox> s_bboxs = subj->create_contour_bboxes();
  std::vector<bbox> c_bboxs = clip->create_contour_bboxes();

  std::vector<bool> s_overlaps(subj->num_contours(), false);

  // 检查辅的轮廓，顺带记录主的轮廓是否有用
  for (int c = 0; c < clip->num_contours(); ++c) {
    bool overlap = false;

    for (int s = 0; s < subj->num_contours(); ++s) {
      if (is_intersect(s_bboxs[s], c_bboxs[c])) {
        overlap = true;
        s_overlaps[s] = true;
      }
    }

    if (!overlap) {
      clip->contour[c].is_contributing = false;
    }
  }

  for (int s = 0; s < subj->num_contours(); ++s) {
    if (!s_overlaps[s]) {
      subj->contour[s].is_contributing = false;
    }
  }
}

void gpc::minimax_test(gpc::gpc_polygon *subj, gpc::gpc_polygon *clip,
                       gpc::gpc_op op) {
  switch (op) {
  case gpc_op::GPC_DIFF:
    minimax_test_diff(subj, clip);
    break;
  case gpc_op::GPC_INT:
    minimax_test_int(subj, clip);
    break;
  }
}