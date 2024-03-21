#pragma once

#include "gpc_vertex.hpp"
#include "gpc_vertex_list.hpp"

namespace gpc {

// Internal contour / tristrip type
class polygon_node
{
public:
    gpc_vertex_list vertex_list;   // Left and right vertex list ptrs
    polygon_node *proxy = nullptr; // Pointer to actual structure used

public:
    polygon_node() = default;
    polygon_node(const gpc_vertex_list &v) : vertex_list(v) {}

    void add_left(const gpc_vertex &v);
    void add_right(const gpc_vertex &v);
};

static void merge_left(polygon_node *p, polygon_node *q,
                       std::vector<polygon_node *> &list)
{
    // Label contour as a hole
    q->proxy->vertex_list.is_hole = true;

    if (p->proxy != q->proxy)
    {
        // Assign p's vertex list to the left end of q's list
        p->proxy->vertex_list.vertexs.insert(
            p->proxy->vertex_list.vertexs.end(),
            q->proxy->vertex_list.vertexs.begin(),
            q->proxy->vertex_list.vertexs.end());

        q->proxy->vertex_list = p->proxy->vertex_list; // TODO:

        // Redirect any p->proxy references to q->proxy
        polygon_node *target = p->proxy;
        for (auto &&polygon : list)
        {
            if (polygon->proxy == target)
            {
                polygon->vertex_list.is_contributing = false;
                polygon->proxy = q->proxy;
            }
        }
    }
}

static void merge_right(polygon_node *p, polygon_node *q,
                        std::vector<polygon_node *> &list)
{
    if (p->proxy != q->proxy)
    {
        // Assign p's vertex list to the right end of q's list
        q->proxy->vertex_list.vertexs.insert(
            q->proxy->vertex_list.vertexs.end(),
            p->proxy->vertex_list.vertexs.begin(),
            p->proxy->vertex_list.vertexs.end());

        // Redirect any p->proxy references to q->proxy
        polygon_node *target = p->proxy;
        for (auto &&polygon : list)
        {
            if (polygon->proxy == target)
            {
                polygon->vertex_list.is_contributing = false;
                polygon->proxy = q->proxy;
            }
        }
    }
}

int count_contours(std::vector<polygon_node *> &polygons);

} // namespace gpc