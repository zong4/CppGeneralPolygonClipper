#include "intersect.hpp"

bool is_intersect(const gpc::gpc_vertex &a1, const gpc::gpc_vertex &a2,
                  const gpc::gpc_vertex &b1, const gpc::gpc_vertex &b2)
{
    double d1 = (b2.x - b1.x) * (a1.y - b1.y) - (b2.y - b1.y) * (a1.x - b1.x);
    double d2 = (b2.x - b1.x) * (a2.y - b1.y) - (b2.y - b1.y) * (a2.x - b1.x);
    double d3 = (a2.x - a1.x) * (b1.y - a1.y) - (a2.y - a1.y) * (b1.x - a1.x);
    double d4 = (a2.x - a1.x) * (b2.y - a1.y) - (a2.y - a1.y) * (b2.x - a1.x);

    return d1 * d2 < 0 && d3 * d4 < 0;
}

std::vector<gpc::gpc_vertex> intersect(const gpc::gpc_vertex &a1,
                                       const gpc::gpc_vertex &a2,
                                       const gpc::gpc_vertex &b1,
                                       const gpc::gpc_vertex &b2)
{
    double d1 = (b2.x - b1.x) * (a1.y - b1.y) - (b2.y - b1.y) * (a1.x - b1.x);
    double d2 = (b2.x - b1.x) * (a2.y - b1.y) - (b2.y - b1.y) * (a2.x - b1.x);
    double d3 = (a2.x - a1.x) * (b1.y - a1.y) - (a2.y - a1.y) * (b1.x - a1.x);
    double d4 = (a2.x - a1.x) * (b2.y - a1.y) - (a2.y - a1.y) * (b2.x - a1.x);

    if (d1 * d2 < 0 && d3 * d4 < 0)
    {
        double u = d1 / (d1 - d2);
        double x = a1.x + u * (a2.x - a1.x);
        double y = a1.y + u * (a2.y - a1.y);
        return {gpc::gpc_vertex(x, y)};
    }
    else
    {
        return {};
    }
}

bool is_intersect(const gpc::gpc_bbox &a, const gpc::gpc_bbox &b)
{
    return (!((a.xmax < b.xmin) || (a.xmin > b.xmax))) &&
           (!((a.ymax < b.ymin) || (a.ymin > b.ymax)));
}
