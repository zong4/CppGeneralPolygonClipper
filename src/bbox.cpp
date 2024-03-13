#include "bbox.hpp"

bool gpc::is_intersect(const gpc::bbox &a, const gpc::bbox &b) {
  return (!((a.xmax < b.xmin) || (a.xmin > b.xmax))) &&
         (!((a.ymax < b.ymin) || (a.ymin > b.ymax)));
}