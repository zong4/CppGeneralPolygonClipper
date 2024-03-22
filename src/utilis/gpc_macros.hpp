#pragma once

#include "../geometry/gpc_vertex_list.hpp"

namespace gpc {

inline int prev_index(int i, int n) { return (i - 1 + n) % n; }
inline int next_index(int i, int n) { return (i + 1) % n; }

// 当前点是否需要被考虑
inline bool optimal(const gpc_vertex_list &v, int i, int n)
{
    return (v.vertexs[prev_index(i, n)].y != v.vertexs[i].y) ||
           (v.vertexs[next_index(i, n)].y != v.vertexs[i].y);
}

// 当前点是否是一个局部最小值, 左闭右开
inline bool fwd_min(const std::vector<gpc_vertex> &v, int i, int n)
{
    return (v[prev_index(i, n)].y >= v[i].y) &&
           (v[next_index(i, n)].y > v[i].y);
}

// 严格大于
inline bool not_fmax(const std::vector<gpc_vertex> &v, int i, int n)
{
    return v[next_index(i, n)].y > v[i].y;
}

// 当前点是否是一个局部最大值, 左开右闭
inline bool rev_min(const std::vector<gpc_vertex> &v, int i, int n)
{
    return (v[prev_index(i, n)].y > v[i].y) &&
           (v[next_index(i, n)].y >= v[i].y);
}

// 严格大于
inline bool not_rmax(const std::vector<gpc_vertex> &v, int i, int n)
{
    return v[prev_index(i, n)].y > v[i].y;
}

} // namespace gpc