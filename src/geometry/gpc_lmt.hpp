#pragma once

#include <list>
#include <vector>

#include "../utilis/gpc_macros.hpp"
#include "gpc_lmt_node.hpp"
#include "gpc_polygon.hpp"

namespace gpc {

class gpc_lmt {
public:
  std::vector<double> sbtree;
  std::list<lmt_node> lmt_list;
  std::vector<edge_node *> edge_tables;

  gpc_lmt() = default;
  ~gpc_lmt();

  void build_lmt(const gpc_polygon &p, int type, gpc_op op);

private:
  // edge_node **bound_list(double y);
  void insert_bound(double y, const edge_node &e);
};

} // namespace gpc