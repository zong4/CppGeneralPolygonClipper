#include "gpc_vertex_list.hpp"

std::istream &gpc::operator>>(std::istream &is,
                              gpc::gpc_vertex_list &vertex_list) {
  int num_vertices;
  is >> num_vertices;

  for (int i = 0; i < num_vertices; ++i) {
    gpc_vertex vertex;
    is >> vertex;
    vertex_list.vertex.push_back(vertex);
  }

  return is;
}

std::ostream &gpc::operator<<(std::ostream &os,
                              const gpc::gpc_vertex_list &vertex_list) {
  os << vertex_list.num_vertices() << "\n";

  for (int i = 0; i < vertex_list.num_vertices(); ++i) {
    os << vertex_list.vertex[i] << "\n";
  }

  return os;
}

gpc::gpc_bbox gpc::gpc_vertex_list::create_bbox() const {
  gpc_bbox bbox;

  if (num_vertices() > 0) {
    bbox.xmin = vertex[0].x;
    bbox.ymin = vertex[0].y;
    bbox.xmax = vertex[0].x;
    bbox.ymax = vertex[0].y;
  }

  for (int i = 1; i < num_vertices(); ++i) {
    bbox.xmin = std::min(bbox.xmin, vertex[i].x);
    bbox.ymin = std::min(bbox.ymin, vertex[i].y);
    bbox.xmax = std::max(bbox.xmax, vertex[i].x);
    bbox.ymax = std::max(bbox.ymax, vertex[i].y);
  }

  return bbox;
}

std::string gpc::gpc_vertex_list::to_string() const {
  std::stringstream ss;

  ss << "gpc_vertex_list: " << num_vertices() << " vertices\n";

  for (int i = 0; i < num_vertices(); ++i) {
    ss << "vertex " << i << ": " << vertex[i].to_string() << "\n";
  }

  return ss.str();
}