#pragma once

// #include <list>

#include "gpc_vertex.hpp"

namespace gpc {

// typedef std::list<gpc_vertex> vertex_node_list;

// TODO: 链表
class vertex_node // Internal vertex list datatype
{
public:
    double x = 0.0;              // X coordinate component
    double y = 0.0;              // Y coordinate component
    vertex_node *next = nullptr; // Pointer to next vertex in list
};

} // namespace gpc