#include "gpc_lmt.hpp"

gpc::gpc_lmt::~gpc_lmt() {
  for (auto &&edge_table : edge_tables) {
    delete edge_table;
  }
  edge_tables.clear();

  for (auto &&lmt : lmt_list) {
    lmt.second.clear();
  }
  lmt_list.clear();

  sbtree.clear();
}

void gpc::gpc_lmt::build_lmt(const gpc_polygon &p, bool type, gpc_op op) {
  int total_vertices = 0;
  for (int c = 0; c < p.num_contours(); ++c) {
    for (int i = 0; i < p.contour[c].num_vertices(); ++i) {
      if (optimal(p.contour[c], i, p.contour[c].num_vertices())) {
        ++total_vertices;
      }
    }
  }

  std::vector<gpc_vertex> vertex_table;

  /* Create the entire input polygon edge table in one go */
  gpc_edge_node *edge_table =
      (gpc_edge_node *)malloc(total_vertices * sizeof(gpc_edge_node));

  int e_index = 0;
  for (int c = 0; c < p.num_contours(); ++c) {
    if (p.contour[c].is_contributing) {
      /* Perform contour optimisation */
      int cnt_vertices = 0;
      for (int i = 0; i < p.contour[c].num_vertices(); ++i) {
        if (optimal(p.contour[c].vertex, i, p.contour[c].num_vertices())) {
          vertex_table.push_back(p.contour[c].vertex[i]);

          // edge_table[cnt_vertices].vertex = p.contour[c].vertex[i];
          ++cnt_vertices;

          /* Record vertex in the scanbeam table */
          sbtree.push_back(p.contour[c].vertex[i].y);
        }
      }

      /* Do the contour forward pass */
      for (int min = 0; min < cnt_vertices; ++min) {
        /* If a forward local minimum... */
        if (fwd_min(vertex_table, min, cnt_vertices)) {
          /* Search for the next local maximum... */
          int num_edges = 1;
          int max = next_index(min, cnt_vertices);

          while (not_fmax(vertex_table, max, cnt_vertices)) {
            ++num_edges;
            max = next_index(max, cnt_vertices);
          }

          /* Build the next edge list */
          edge_table[e_index].bstate[BELOW] = bundle_state::UNBUNDLED;
          edge_table[e_index].bundle[BELOW][CLIP] = FALSE;
          edge_table[e_index].bundle[BELOW][SUBJ] = FALSE;

          int v = min;
          for (int i = 0; i < num_edges; ++i) {
            edge_table[e_index + i].xb = vertex_table[v].x;
            edge_table[e_index + i].bot = vertex_table[v];

            v = next_index(v, cnt_vertices);

            edge_table[e_index + i].top = vertex_table[v];

            edge_table[e_index + i].dx =
                (vertex_table[v].x - edge_table[e_index + i].bot.x) /
                (edge_table[e_index + i].top.y - edge_table[e_index + i].bot.y);

            edge_table[e_index + i].type = type;

            edge_table[e_index + i].succ =
                ((num_edges > 1) && (i < (num_edges - 1)))
                    ? &(edge_table[e_index + i + 1])
                    : nullptr;

            edge_table[e_index + i].pred = ((num_edges > 1) && (i > 0))
                                               ? &(edge_table[e_index + i - 1])
                                               : nullptr;

            edge_table[e_index + i].bside[CLIP] =
                (op == gpc_op::GPC_DIFF) ? RIGHT : LEFT;
            edge_table[e_index + i].bside[SUBJ] = LEFT;
          }

          insert_bound(vertex_table[min].y, edge_table[e_index]);

          e_index += num_edges;
        }
      }

      /* Do the contour reverse pass */
      for (int min = 0; min < cnt_vertices; ++min) {
        /* If a reverse local minimum... */
        if (rev_min(vertex_table, min, cnt_vertices)) {
          /* Search for the previous local maximum... */
          int num_edges = 1;
          int max = prev_index(min, cnt_vertices);

          while (not_rmax(vertex_table, max, cnt_vertices)) {
            ++num_edges;
            max = prev_index(max, cnt_vertices);
          }

          /* Build the previous edge list */
          edge_table[e_index].bstate[BELOW] = bundle_state::UNBUNDLED;
          edge_table[e_index].bundle[BELOW][CLIP] = FALSE;
          edge_table[e_index].bundle[BELOW][SUBJ] = FALSE;

          int v = min;
          for (int i = 0; i < num_edges; i++) {
            edge_table[e_index + i].xb = vertex_table[v].x;
            edge_table[e_index + i].bot = vertex_table[v];

            v = prev_index(v, cnt_vertices);

            edge_table[e_index + i].top = vertex_table[v];

            edge_table[e_index + i].dx =
                (vertex_table[v].x - edge_table[e_index + i].bot.x) /
                (edge_table[e_index + i].top.y - edge_table[e_index + i].bot.y);

            edge_table[e_index + i].type = type;

            edge_table[e_index + i].succ =
                ((num_edges > 1) && (i < (num_edges - 1)))
                    ? &(edge_table[e_index + i + 1])
                    : nullptr;

            edge_table[e_index + i].pred = ((num_edges > 1) && (i > 0))
                                               ? &(edge_table[e_index + i - 1])
                                               : nullptr;

            edge_table[e_index + i].bside[CLIP] =
                (op == gpc_op::GPC_DIFF) ? RIGHT : LEFT;
            edge_table[e_index + i].bside[SUBJ] = LEFT;
          }

          // edge_node *e = &edge_table[e_index];
          insert_bound(vertex_table[min].y, edge_table[e_index]);

          e_index += num_edges;
        }
      }
    }
  }

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