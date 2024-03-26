#include "extend.hpp"
#include <cmath>
#include <vector>
#include "intersect.hpp"

gpc::gpc_polygon extend(const gpc::gpc_polygon &polygon, double len)
{
    int flag = 1;

    gpc::gpc_vertex last_vertex;
    gpc::gpc_polygon result;
    for (auto &&contour : polygon.contours)
    {
        gpc::gpc_vertex_list vertex_list;
        for (int j = 0; j < contour.num_vertices(); ++j)
        {
            gpc::gpc_vertex v1 =
                contour.vertexs[(j + 1) % contour.num_vertices()] -
                contour.vertexs[j];
            gpc::gpc_vertex v2 =
                contour.vertexs[(j - 1 + contour.num_vertices()) %
                                contour.num_vertices()] -
                contour.vertexs[j];

            double theta = std::acos(v1 * v2 / (v1.norm() * v2.norm()));
            double len_real = len / std::sin(theta * 0.5);

            v1.rotate90();
            v1.normalize();

            v2.rotate90_reverse();
            v2.normalize();

            gpc::gpc_vertex v = v1 + v2;
            v.normalize();

            gpc::gpc_vertex vertex_temp =
                contour.vertexs[j] + v * len_real * flag;

            if (j == 0)
            {
                vertex_list.vertexs.push_back(vertex_temp);
                last_vertex = vertex_temp;
                continue;
            }

            if (is_intersect(last_vertex,
                             contour.vertexs[(j - 1 + contour.num_vertices()) %
                                             contour.num_vertices()],
                             vertex_temp, contour.vertexs[j]))
            {
                // vertex_list.vertexs.push_back(
                //     intersect(last_vertex,
                //               contour.vertexs[(j - 1 +
                //               contour.num_vertices()) %
                //                               contour.num_vertices()],
                //               vertex_temp, contour.vertexs[j])[0]);
                last_vertex = vertex_temp;
            }
            else
            {
                vertex_list.vertexs.push_back(vertex_temp);
                last_vertex = vertex_temp;
            }
        }

        result.contours.push_back(vertex_list);
    }

    return result;
}
