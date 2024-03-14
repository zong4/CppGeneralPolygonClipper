#pragma once

#include "vertex_node.hpp"

namespace gpc {

// TODO:
class polygon_node /* Internal contour / tristrip type  */
{
public:
  int active = 0;                /* Active flag / vertex count        */
  int hole = 0;                  /* Hole / external contour flag      */
  vertex_node *v[2];             /* Left and right vertex list ptrs   */
  polygon_node *next = nullptr;  /* Pointer to next polygon contour   */
  polygon_node *proxy = nullptr; /* Pointer to actual structure used  */
};

} // namespace gpc