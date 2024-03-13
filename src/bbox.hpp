#pragma once

namespace gpc {

class bbox /* Contour axis-aligned bounding box */
{
public:
  double xmin = 0.0; /* Minimum x coordinate              */
  double ymin = 0.0; /* Minimum y coordinate              */
  double xmax = 0.0; /* Maximum x coordinate              */
  double ymax = 0.0; /* Maximum y coordinate              */
};

bool is_intersect(const bbox &a, const bbox &b);

} // namespace gpc