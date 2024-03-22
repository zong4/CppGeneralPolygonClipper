#pragma once

#include "gpc_edge_node.hpp"

namespace gpc {

// Intersection table
class it_node
{
public:
    gpc_vertex point;     // Point of intersection
    gpc_edge_node *ie[2]; // Intersecting edge (bundle) pair

public:
    it_node(gpc_edge_node *edge0, gpc_edge_node *edge1, double x, double y)
        : point(x, y)
    {
        ie[0] = edge0;
        ie[1] = edge1;
    }
};

}; // namespace gpc