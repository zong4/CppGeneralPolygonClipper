#pragma once

// TODO: 链表
#include <list>

#include "gpc_vertex.hpp"

namespace gpc {

// Internal vertex list datatype
class vertex_node
{
public:
    double x = 0.0;              // X coordinate component
    double y = 0.0;              // Y coordinate component
    vertex_node *next = nullptr; // Pointer to next vertex in list
};

typedef std::list<gpc_vertex> vertex_node_list;

} // namespace gpc