#pragma once

#include "gpc_vertex_list.hpp"

namespace gpc {

/* Tristrip set structure            */
class gpc_tristrip {
public:
  std::vector<gpc_vertex_list> strip; /* Tristrip array pointer            */
};

} // namespace gpc
