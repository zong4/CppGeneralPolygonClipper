#pragma once

#include "edge_node.hpp"
#include "gpc_macros.hpp"
#include "gpc_vertex_list.hpp"

namespace gpc {

/*
===========================================================================
                                Constants
===========================================================================
*/

#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif

#define LEFT 0
#define RIGHT 1

#define ABOVE 0
#define BELOW 1

#define CLIP 0
#define SUBJ 1

#define INVERT_TRISTRIPS FALSE

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

// TODO: 转换成函数

#define VERTEX(e, p, s, x, y)                                                  \
  {                                                                            \
    add_vertex(&((e)->outp[(p)]->v[(s)]), x, y);                               \
    (e)->outp[(p)]->active++;                                                  \
  }

#define P_EDGE(d, e, p, i, j)                                                  \
  {                                                                            \
    (d) = (e);                                                                 \
    do {                                                                       \
      (d) = (d)->prev;                                                         \
    } while (!(d)->outp[(p)]);                                                 \
    (i) = (d)->bot.x + (d)->dx * ((j) - (d)->bot.y);                           \
  }

#define N_EDGE(d, e, p, i, j)                                                  \
  {                                                                            \
    (d) = (e);                                                                 \
    do {                                                                       \
      (d) = (d)->next;                                                         \
    } while (!(d)->outp[(p)]);                                                 \
    (i) = (d)->bot.x + (d)->dx * ((j) - (d)->bot.y);                           \
  }

#define MALLOC(p, b, s, t)                                                     \
  {                                                                            \
    if ((b) > 0) {                                                             \
      p = (t *)malloc(b);                                                      \
      if (!(p)) {                                                              \
        fprintf(stderr, "gpc malloc failure: %s\n", s);                        \
        exit(0);                                                               \
      }                                                                        \
    } else                                                                     \
      p = NULL;                                                                \
  }

} // namespace gpc