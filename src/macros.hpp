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

/*
===========================================================================
                                 Macros
===========================================================================
*/

#define EQ(a, b) (fabs((a) - (b)) <= GPC_EPSILON)

#define PREV_INDEX(i, n) ((i - 1 + n) % n)
#define NEXT_INDEX(i, n) ((i + 1) % n)

#define FWD_MIN(v, i, n)                                                       \
  ((v[PREV_INDEX(i, n)].vertex.y >= v[i].vertex.y) &&                          \
   (v[NEXT_INDEX(i, n)].vertex.y > v[i].vertex.y))

#define NOT_FMAX(v, i, n) (v[NEXT_INDEX(i, n)].vertex.y > v[i].vertex.y)

#define REV_MIN(v, i, n)                                                       \
  ((v[PREV_INDEX(i, n)].vertex.y > v[i].vertex.y) &&                           \
   (v[NEXT_INDEX(i, n)].vertex.y >= v[i].vertex.y))

#define NOT_RMAX(v, i, n) (v[PREV_INDEX(i, n)].vertex.y > v[i].vertex.y)

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