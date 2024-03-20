#pragma once

#include <cmath>
#include <float.h>

namespace gpc {

// Increase GPC_EPSILON to encourage merging of near coincident edges
const double GPC_EPSILON = DBL_EPSILON;

inline bool equal(double a, double b)
{
    return (fabs((a) - (b)) <= GPC_EPSILON);
}

} // namespace gpc