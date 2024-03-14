#include "utilis.hpp"

bool gpc::optimal(gpc::gpc_vertex_list const &v, int i, int n) {
  return (v.vertex[PREV_INDEX(i, n)].y != v.vertex[i].y) ||
         (v.vertex[NEXT_INDEX(i, n)].y != v.vertex[i].y);
}