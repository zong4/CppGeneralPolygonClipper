#pragma once

#include <vector>

#include "../geometry/gpc_vertex.hpp"
#include "../geometry/gpc_bbox.hpp"

bool is_intersect(const gpc::gpc_vertex &a1, const gpc::gpc_vertex &a2,
                  const gpc::gpc_vertex &b1, const gpc::gpc_vertex &b2);

std::vector<gpc::gpc_vertex> intersect(const gpc::gpc_vertex &a1,
                                       const gpc::gpc_vertex &a2,
                                       const gpc::gpc_vertex &b1,
                                       const gpc::gpc_vertex &b2);

bool is_intersect(const gpc::gpc_bbox &a, const gpc::gpc_bbox &b);
