#pragma once

namespace gpc {

class gpc_bbox /* Contour axis-aligned bounding box */
{
public:
  double xmin = 0.0; /* Minimum x coordinate              */
  double ymin = 0.0; /* Minimum y coordinate              */
  double xmax = 0.0; /* Maximum x coordinate              */
  double ymax = 0.0; /* Maximum y coordinate              */
};

bool is_intersect(const gpc_bbox &a, const gpc_bbox &b);

} // namespace gpc