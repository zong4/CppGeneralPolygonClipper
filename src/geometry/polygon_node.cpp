#include "polygon_node.hpp"

void gpc::polygon_node::add_left(const gpc_vertex &v)
{
    // Add vertex v to the left end of the polygon's vertex list
    proxy->vertex_list.vertexs.insert(proxy->vertex_list.vertexs.begin(), v);
}

void gpc::polygon_node::add_right(const gpc_vertex &v)
{
    // Add vertex v to the right end of the polygon's vertex list
    proxy->vertex_list.vertexs.push_back(v);
}

int gpc::count_contours(std::vector<gpc::polygon_node *> &polygons)
{
    int nc = 0;
    for (auto &&polygon : polygons)
    {
        if (polygon->vertex_list.is_contributing)
        {
            // Count the vertices in the current contour
            int nv = polygon->proxy->vertex_list.num_vertices();

            // Record valid vertex counts in the active field
            if (nv > 2)
            {
                polygon->vertex_list.is_contributing = true;
                ++nc;
            }
            else
            {
                // Invalid contour: just free the heap
                polygon->vertex_list.is_contributing = false;
                polygon->proxy->vertex_list.vertexs.clear();
            }
        }
    }
    return nc;
}