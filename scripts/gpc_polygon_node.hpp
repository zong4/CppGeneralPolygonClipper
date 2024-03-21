#pragma once

#include "vertex_node.hpp"
#include "../utilis/gpc_constants.hpp"

namespace gpc {

// TODO:
// Internal contour / tristrip type
class gpc_polygon_node
{
public:
    bool active = false;          // Active flag / vertex count
    bool hole = false;            // Hole / external contour flag
    vertex_node_list vertex_list; // Left and right vertex list ptrs
    // gpc_polygon_node *proxy = nullptr; // Pointer to actual structure used

public:
    gpc_polygon_node() = default;
    gpc_polygon_node(const gpc_vertex &v, int hole_flag = false,
                     int active_flag = true);
    gpc_polygon_node(const gpc_polygon_node &rhs) = default;
    ~gpc_polygon_node() = default;

    inline bool operator==(const gpc_polygon_node &rhs) const
    {
        return (vertex_list == rhs.vertex_list) && (hole == rhs.hole);
    }

    void add_left(const gpc_vertex &v);
    void add_right(const gpc_vertex &v);
    // void add_local_min(const gpc_vertex &v);
};

typedef std::list<gpc_polygon_node> gpc_polygon_node_list;

void merge_left(gpc_polygon_node &p, gpc_polygon_node &q,
                std::list<gpc_polygon_node> &list);
void merge_right(gpc_polygon_node &p, gpc_polygon_node &q,
                 std::list<gpc_polygon_node> &list);

} // namespace gpc