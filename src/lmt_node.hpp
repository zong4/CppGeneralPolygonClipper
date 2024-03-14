#pragma once

#include "edge_node.hpp"

namespace gpc {

// TODO:
typedef std::pair<double, edge_node *> lmt_node;

// class lmt_node {
// public:
//   double y = 0.0;
//   edge_node *first_bound = nullptr;

//   lmt_node() = default;
//   lmt_node(double y, edge_node *first_bound) : y(y), first_bound(first_bound)
//   {} ~lmt_node() { delete first_bound; }
// };

} // namespace gpc
