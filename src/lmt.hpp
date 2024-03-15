#pragma once

#include <list>
#include <vector>

#include "gpc_polygon.hpp"
#include "lmt_node.hpp"
#include "utilis.hpp"

namespace gpc {

class Lmt {
public:
  std::vector<double> sbtree;
  std::list<lmt_node> lmt_list;
  std::vector<edge_node *> edge_tables;

  Lmt() = default;
  ~Lmt();

  void build_lmt(gpc_polygon *p, int type, gpc_op op);

private:
  edge_node **bound_list(double y);
};

} // namespace gpc