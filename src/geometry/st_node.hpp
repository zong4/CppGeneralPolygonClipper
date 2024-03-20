#pragma once

#include "gpc_edge_node.hpp"

namespace gpc {

class st_node // Sorted edge table
{
public:
    gpc_edge_node *edge; // Pointer to AET edge
    double xb;           // Scanbeam bottom x coordinate
    double xt;           // Scanbeam top x coordinate
    double dx;           // Change in x for a unit y increase
    st_node *prev;       // Previous edge in sorted list
};

} // namespace gpc