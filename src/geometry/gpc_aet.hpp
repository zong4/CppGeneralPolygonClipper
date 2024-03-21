#pragma once

#include <list>

#include "gpc_edge_node.hpp"

namespace gpc {

class gpc_aet
{
public:
    std::list<gpc_edge_node> aet_list; // TODO: ТаЉ

public:
    gpc_aet() = default;
    ~gpc_aet() = default;

    void insert(const gpc_edge_node &edge);
};

} // namespace gpc