#pragma once

#include "edge_node.hpp"
#include "gpc_macros.hpp"
#include "gpc_vertex_list.hpp"

namespace gpc {

inline int prev_index(int i, int n) { return (i - 1 + n) % n; }
inline int next_index(int i, int n) { return (i + 1) % n; }

// 当前点是否需要被考虑
inline bool optimal(const gpc_vertex_list &v, int i, int n) {
  return (v.vertex[prev_index(i, n)].y != v.vertex[i].y) ||
         (v.vertex[next_index(i, n)].y != v.vertex[i].y);
}

// 当前点是否是一个局部最小值, 左闭右开
inline bool fwd_min(edge_node *v, int i, int n) {
  return (v[prev_index(i, n)].vertex.y >= v[i].vertex.y) &&
         (v[next_index(i, n)].vertex.y > v[i].vertex.y);
}

// 严格大于
inline bool not_fmax(edge_node *v, int i, int n) {
  return v[next_index(i, n)].vertex.y > v[i].vertex.y;
}

// 当前点是否是一个局部最大值, 左开右闭
inline bool rev_min(edge_node *v, int i, int n) {
  return (v[prev_index(i, n)].vertex.y > v[i].vertex.y) &&
         (v[next_index(i, n)].vertex.y >= v[i].vertex.y);
}

// 严格大于
inline bool not_rmax(edge_node *v, int i, int n) {
  return v[prev_index(i, n)].vertex.y > v[i].vertex.y;
}

} // namespace gpc
