#pragma once

#include "gpc_vertex_list.hpp"

namespace gpc {

struct gpc_tristrip /* Tristrip set structure            */
{
  int num_strips = 0;               /* Number of tristrips               */
  gpc_vertex_list *strip = nullptr; /* Tristrip array pointer            */
};

} // namespace gpc
