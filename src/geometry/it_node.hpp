#pragma once

#include "gpc_edge_node.hpp"

namespace gpc {

class it_node // Intersection table
{
public:
    gpc_edge_node *ie[2];    // Intersecting edge (bundle) pair
    gpc_vertex point;        // Point of intersection
    it_node *next = nullptr; // The next intersection table node
};

}; // namespace gpc