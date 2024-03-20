#include "gpc_aet.hpp"

void gpc::gpc_aet::insert(const gpc_edge_node &edge) {
  if (aet_list.empty()) {
    aet_list.push_back(edge);
    return;
  }

  for (auto it = aet_list.begin(); it != aet_list.end(); ++it) {
    if (edge.xb() < it->xb()) {
      aet_list.insert(it, edge);
      return;
    } else if (edge.xb() == it->xb()) {
      if (edge.dx < it->dx) {
        aet_list.insert(it, edge);
        return;
      }
    }
  }

  aet_list.push_back(edge);
}