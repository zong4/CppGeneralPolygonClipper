#pragma once

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