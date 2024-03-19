#include "gpc_lmt.hpp"

gpc::gpc_lmt::~gpc_lmt() {
  // for (auto &&edge_table : edge_tables) {
  //   for (auto &&edge : edge_table) {
  //     edge.outp[0] = nullptr;
  //     edge.outp[1] = nullptr;
  //     edge.prev = nullptr;
  //     edge.next = nullptr;
  //     edge.pred = nullptr;
  //     edge.succ = nullptr;
  //   }
  //   edge_table.clear();
  // }
  // edge_tables.clear();

  for (auto &&lmt : lmt_list) {
    lmt.second.clear();
  }
  lmt_list.clear();

  sbtree.clear();
}

void gpc::gpc_lmt::build_lmt(const gpc_polygon &p, bool type, gpc_op op) {
  for (int c = 0; c < p.num_contours(); ++c) {
    if (!p.contour[c].is_contributing) {
      continue;
    }

    std::vector<gpc_vertex> vertex_table;

    /* Perform contour optimisation */
    for (int i = 0; i < p.contour[c].num_vertices(); ++i) {
      if (optimal(p.contour[c].vertex, i, p.contour[c].num_vertices())) {
        vertex_table.push_back(p.contour[c].vertex[i]);

        /* Record vertex in the scanbeam table */
        sbtree.push_back(p.contour[c].vertex[i].y);
      }
    }

    /* Do the contour forward pass */
    for (int min = 0; min < vertex_table.size(); ++min) {
      /* If a forward local minimum... */
      if (fwd_min(vertex_table, min, vertex_table.size())) {
        /* Search for the next local maximum... */
        int num_edges = 1;
        int max = next_index(min, vertex_table.size());

        while (not_fmax(vertex_table, max, vertex_table.size())) {
          ++num_edges;
          max = next_index(max, vertex_table.size());
        }

        set_edge_table(min, num_edges, vertex_table, type, op, false);
      }
    }

    /* Do the contour reverse pass */
    for (int min = 0; min < vertex_table.size(); ++min) {
      /* If a reverse local minimum... */
      if (rev_min(vertex_table, min, vertex_table.size())) {
        /* Search for the previous local maximum... */
        int num_edges = 1;
        int max = prev_index(min, vertex_table.size());

        while (not_rmax(vertex_table, max, vertex_table.size())) {
          ++num_edges;
          max = prev_index(max, vertex_table.size());
        }

        set_edge_table(min, num_edges, vertex_table, type, op, true);
      }
    }
  }
}

void gpc::gpc_lmt::set_edge_table(int min, int num_edges,
                                  const std::vector<gpc_vertex> &vertex_table,
                                  bool type, gpc_op op, bool is_reverse) {
  /* Create the entire input polygon edge table in one go */
  // std::vector<gpc_edge_node> edge_table(vertex_table.size());
  gpc_edge_node *edge_table =
      (gpc_edge_node *)malloc(vertex_table.size() * sizeof(gpc_edge_node));

  /* Build the next edge list */
  edge_table[min].bstate[BELOW] = bundle_state::UNBUNDLED;
  edge_table[min].bundle[BELOW][CLIP] = FALSE;
  edge_table[min].bundle[BELOW][SUBJ] = FALSE;

  int v = min;
  for (int i = 0; i < num_edges; ++i) {
    edge_table[min + i].bot = vertex_table[v];

    v = is_reverse ? prev_index(v, vertex_table.size())
                   : next_index(v, vertex_table.size());

    edge_table[min + i].top = vertex_table[v];

    edge_table[min + i].xb(edge_table[min + i].bot.x);
    edge_table[min + i].xt(edge_table[min + i].top.x);

    edge_table[min + i].dx = (vertex_table[v].x - vertex_table[min + i].x) /
                             (vertex_table[v].y - vertex_table[min + i].y);

    edge_table[min + i].type = type;

    edge_table[min + i].succ = (num_edges > 1 && i < num_edges - 1)
                                   ? &(edge_table[min + i + 1])
                                   : nullptr;

    edge_table[min + i].pred =
        ((num_edges > 1) && (i > 0)) ? &(edge_table[min + i - 1]) : nullptr;

    edge_table[min + i].bside[CLIP] = (op == gpc_op::GPC_DIFF) ? RIGHT : LEFT;
    edge_table[min + i].bside[SUBJ] = LEFT;
  }

  insert_bound(vertex_table[min].y, edge_table[min]);

  edge_tables.push_back(edge_table);
}

void gpc::gpc_lmt::insert_bound(double y, const gpc_edge_node &e) {
  if (lmt_list.empty()) {
    /* Add node onto the tail end of the LMT */
    lmt_list.push_back(gpc_lmt_node(y, {e}));
    return;
  }

  for (auto it = lmt_list.begin(); it != lmt_list.end(); ++it) {
    if (y < it->first) {
      /* Insert a new LMT node before the current node */
      lmt_list.insert(it, gpc_lmt_node(y, {e}));
      return;
    } else if (y == it->first) {
      for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
        if (e.bot.x < it2->bot.x) {
          /* Insert a new node mid-list */
          it->second.insert(it2, e);
          return;
        } else if (e.bot.x == it2->bot.x) {
          if (e.dx < it2->dx) {
            /* Insert a new node mid-list */
            it->second.insert(it2, e);
            return;
          }
        }
      }

      return;
    }
  }

  /* Add node onto the tail end of the LMT */
  lmt_list.push_back(gpc_lmt_node(y, {e}));
  return;
}