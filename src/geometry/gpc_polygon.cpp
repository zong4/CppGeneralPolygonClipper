#include "gpc_polygon.hpp"

gpc::gpc_polygon::gpc_polygon(
    const std::vector<std::vector<gpc_vertex>> &in_contour)
{
    for (int i = 0; i < in_contour.size(); ++i)
    {
        contours.push_back(gpc_vertex_list(in_contour[i]));
    }
}

bool gpc::gpc_polygon::operator==(const gpc::gpc_polygon &rhs) const
{
    if (num_contours() != rhs.num_contours())
    {
        return false;
    }

    for (int i = 0; i < num_contours(); ++i)
    {
        if (contours[i] != rhs.contours[i])
        {
            return false;
        }
    }

    return true;
}

std::istream &gpc::operator>>(std::istream &is, gpc::gpc_polygon &polygon)
{
    int num_contours;
    is >> num_contours;

    for (int i = 0; i < num_contours; ++i)
    {
        gpc_vertex_list vertex_list;
        is >> vertex_list;

        if (vertex_list.num_vertices() == 0)
        {
            continue;
        }

        polygon.add_contour(vertex_list);
    }

    return is;
}

std::ostream &gpc::operator<<(std::ostream &os, const gpc::gpc_polygon &polygon)
{
    os << polygon.num_contours() << "\n";

    for (int i = 0; i < polygon.num_contours(); ++i)
    {
        os << polygon.contours[i];
    }

    // os << polygon.to_string();

    return os;
}

void gpc::gpc_polygon::add_contour(const gpc_vertex_list &in_contour)
{
    contours.push_back(in_contour);
}

std::string gpc::gpc_polygon::to_string() const
{
    std::stringstream ss;

    ss << "gpc_polygon: " << num_contours() << " contours\n";

    for (int i = 0; i < num_contours(); ++i)
    {
        ss << "contour " << i << ":\n";
        ss << contours[i].to_string();
    }

    return ss.str();
}

std::vector<gpc::gpc_bbox> gpc::gpc_polygon::create_contour_bboxes() const
{
    std::vector<gpc_bbox> bboxes;

    for (int i = 0; i < num_contours(); ++i)
    {
        bboxes.push_back(contours[i].create_bbox());
    }

    return bboxes;
}

void gpc::minimax_test_diff(const gpc::gpc_polygon &subj,
                            gpc::gpc_polygon &clip)
{
    std::vector<gpc_bbox> s_bboxs = subj.create_contour_bboxes();
    std::vector<gpc_bbox> c_bboxs = clip.create_contour_bboxes();

    // 主的轮廓都有用，只需要检查辅的轮廓
    for (int c = 0; c < clip.num_contours(); ++c)
    {
        bool overlap = false;

        for (int s = 0; s < subj.num_contours(); ++s)
        {
            if (is_intersect(s_bboxs[s], c_bboxs[c]))
            {
                overlap = true;
            }
        }

        if (!overlap)
        {
            // Flag non contributing status by negating vertex count
            clip.contours[c].is_contributing = false;
        }
    }
}

void gpc::minimax_test_int(gpc::gpc_polygon &subj, gpc::gpc_polygon &clip)
{
    std::vector<gpc_bbox> s_bboxs = subj.create_contour_bboxes();
    std::vector<gpc_bbox> c_bboxs = clip.create_contour_bboxes();

    std::vector<bool> s_overlaps(subj.num_contours(), false);

    // 检查辅的轮廓，顺带记录主的轮廓是否有用
    for (int c = 0; c < clip.num_contours(); ++c)
    {
        bool overlap = false;

        for (int s = 0; s < subj.num_contours(); ++s)
        {
            if (is_intersect(s_bboxs[s], c_bboxs[c]))
            {
                overlap = true;
                s_overlaps[s] = true;
            }
        }

        if (!overlap)
        {
            clip.contours[c].is_contributing = false;
        }
    }

    for (int s = 0; s < subj.num_contours(); ++s)
    {
        if (!s_overlaps[s])
        {
            subj.contours[s].is_contributing = false;
        }
    }
}

void gpc::minimax_test(gpc::gpc_polygon &subj, gpc::gpc_polygon &clip,
                       gpc::gpc_op op)
{
    switch (op)
    {
        case gpc_op::GPC_DIFF: minimax_test_diff(subj, clip); break;
        case gpc_op::GPC_INT: minimax_test_int(subj, clip); break;
    }
}

bool gpc::equal_sort(const gpc::gpc_polygon &subj, const gpc::gpc_polygon &clip)
{
    if (subj.num_contours() != clip.num_contours())
    {
        return false;
    }

    for (int i = 0; i < subj.num_contours(); ++i)
    {
        std::vector<gpc_vertex> subj_vertex = subj.contours[i].vertexs;
        std::sort(subj_vertex.begin(), subj_vertex.end(),
                  [](const gpc_vertex &a, const gpc_vertex &b) {
                      if (a.x != b.x)
                          return a.x < b.x;
                      else
                          return a.y < b.y;
                  });

        std::vector<gpc_vertex> clip_vertex = clip.contours[i].vertexs;
        std::sort(clip_vertex.begin(), clip_vertex.end(),
                  [](const gpc_vertex &a, const gpc_vertex &b) {
                      if (a.x != b.x)
                          return a.x < b.x;
                      else
                          return a.y < b.y;
                  });

        for (int j = 0; j < subj_vertex.size(); ++j)
        {
            if (subj_vertex[j] != clip_vertex[j])
            {
                return false;
            }
        }
    }

    return true;
}