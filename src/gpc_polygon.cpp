#include "gpc_polygon.hpp"

std::istream &gpc::operator>>(std::istream &is, gpc::gpc_polygon &polygon) {
  int num_contours;
  is >> num_contours;

  for (int i = 0; i < num_contours; ++i) {
    bool hole;
    is >> hole;

    gpc_vertex_list vertex_list;
    is >> vertex_list;

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