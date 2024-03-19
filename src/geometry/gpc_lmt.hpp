#pragma once

#include <list>
#include <vector>

#include "../utilis/gpc_macros.hpp"
#include "gpc_edge_node.hpp"
#include "gpc_polygon.hpp"

namespace gpc {

typedef std::pair<double, std::list<gpc_edge_node>> gpc_lmt_node;

class gpc_lmt {
public:
  std::vector<double> sbtree;
  std::list<gpc_lmt_node> lmt_list;
  std::vector<gpc_edge_node *> edge_tables;

  gpc_lmt() = default;
  ~gpc_lmt();

  void build_lmt(const gpc_polygon &p, bool type, gpc_op op);

private:
  // edge_node **bound_list(double y);
  void insert_bound(double y, const gpc_edge_node &e);
};

} // namespace gpc