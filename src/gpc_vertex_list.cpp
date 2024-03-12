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

void gpc::gpc_vertex_list::optimal() {}

std::string gpc::gpc_vertex_list::to_string() const {
  std::stringstream ss;

  ss << "gpc_vertex_list: " << num_vertices() << " vertices\n";

  for (int i = 0; i < num_vertices(); ++i) {
    ss << "vertex " << i << ": " << vertex[i].to_string() << "\n";
  }

  return ss.str();
}
