#include "gpc_bbox.hpp"

bool gpc::is_intersect(const gpc::gpc_bbox &a, const gpc::gpc_bbox &b) {
  return (!((a.xmax < b.xmin) || (a.xmin > b.xmax))) &&
         (!((a.ymax < b.ymin) || (a.ymin > b.ymax)));
}