#include "Lmt.hpp"

void gpc::insert_bound(gpc::edge_node **b, gpc::edge_node *e) {
  edge_node *existing_bound;

  if (!*b) {
    /* Link node e to the tail of the list */
    *b = e;
  } else {
    /* Do primary sort on the x field */
    if (e->bot.x < (*b)->bot.x) {
      /* Insert a new node mid-list */
      existing_bound = *b;
      *b = e;
      (*b)->next_bound = existing_bound;
    } else {
      if (e->bot.x == (*b)->bot.x) {
        /* Do secondary sort on the dx field */
        if (e->dx < (*b)->dx) {
          /* Insert a new node mid-list */
          existing_bound = *b;
          *b = e;
          (*b)->next_bound = existing_bound;
        } else {
          /* Head further down the list */
          insert_bound(&((*b)->next_bound), e);
        }
      } else {
        /* Head further down the list */
        insert_bound(&((*b)->next_bound), e);
      }
    }
  }
}

gpc::Lmt::~Lmt() {
  for (auto &&edge_table : edge_tables) {
    delete edge_table;
  }
  edge_tables.clear();

  for (auto &&lmt : lmt_list) {
    delete lmt.second;
  }
  lmt_list.clear();

  sbtree.clear();
}

void gpc::Lmt::build_lmt(gpc_polygon *p, int type, gpc_op op) {
  int total_vertices = 0;
  for (int c = 0; c < p->num_contours(); ++c) {
    for (int i = 0; i < p->contour[c].num_vertices(); ++i) {
      if (optimal(p->contour[c], i, p->contour[c].num_vertices())) {
        ++total_vertices;
      }
    }
  }

  edge_node *edge_table;
  /* Create the entire input polygon edge table in one go */
  // std::vector<edge_node> edge_table(total_vertices);
  MALLOC(edge_table, total_vertices * sizeof(edge_node), "edge table creation",
         edge_node);

  int e_index = 0;
  for (int c = 0; c < p->num_contours(); ++c) {
    if (p->contour[c].is_contributing) {
      /* Perform contour optimisation */
      int cnt_vertices = 0;
      for (int i = 0; i < p->contour[c].num_vertices(); ++i)
        if (optimal(p->contour[c].vertex, i, p->contour[c].num_vertices())) {
          edge_table[cnt_vertices].vertex = p->contour[c].vertex[i];
          ++cnt_vertices;

          /* Record vertex in the scanbeam table */
          sbtree.push_back(p->contour[c].vertex[i].y);
        }

      /* Do the contour forward pass */
      for (int min = 0; min < cnt_vertices; ++min) {
        /* If a forward local minimum... */
        if (FWD_MIN(edge_table, min, cnt_vertices)) {
          /* Search for the next local maximum... */
          int num_edges = 1;
          int max = NEXT_INDEX(min, cnt_vertices);

          while (NOT_FMAX(edge_table, max, cnt_vertices)) {
            ++num_edges;
            max = NEXT_INDEX(max, cnt_vertices);
          }

          /* Build the next edge list */
          edge_table[e_index].bstate[BELOW] = bundle_state::UNBUNDLED;
          edge_table[e_index].bundle[BELOW][CLIP] = FALSE;
          edge_table[e_index].bundle[BELOW][SUBJ] = FALSE;

          int v = min;
          for (int i = 0; i < num_edges; ++i) {
            edge_table[e_index + i].xb = edge_table[v].vertex.x;
            edge_table[e_index + i].bot.x = edge_table[v].vertex.x;
            edge_table[e_index + i].bot.y = edge_table[v].vertex.y;

            v = NEXT_INDEX(v, cnt_vertices);

            edge_table[e_index + i].top.x = edge_table[v].vertex.x;
            edge_table[e_index + i].top.y = edge_table[v].vertex.y;
            edge_table[e_index + i].dx =
                (edge_table[v].vertex.x - edge_table[e_index + i].bot.x) /
                (edge_table[e_index + i].top.y - edge_table[e_index + i].bot.y);
            edge_table[e_index + i].type = type;
            edge_table[e_index + i].outp[ABOVE] = nullptr;
            edge_table[e_index + i].outp[BELOW] = nullptr;
            edge_table[e_index + i].next = nullptr;
            edge_table[e_index + i].prev = nullptr;
            edge_table[e_index + i].succ =
                ((num_edges > 1) && (i < (num_edges - 1)))
                    ? &(edge_table[e_index + i + 1])
                    : nullptr;
            edge_table[e_index + i].pred = ((num_edges > 1) && (i > 0))
                                               ? &(edge_table[e_index + i - 1])
                                               : nullptr;
            edge_table[e_index + i].next_bound = nullptr;
            edge_table[e_index + i].bside[CLIP] =
                (op == gpc_op::GPC_DIFF) ? RIGHT : LEFT;
            edge_table[e_index + i].bside[SUBJ] = LEFT;
          }

          // edge_node *e = &edge_table[e_index];
          insert_bound(bound_list(edge_table[min].vertex.y),
                       &edge_table[e_index]);

          e_index += num_edges;
        }
      }

      /* Do the contour reverse pass */
      for (int min = 0; min < cnt_vertices; ++min) {
        /* If a reverse local minimum... */
        if (REV_MIN(edge_table, min, cnt_vertices)) {
          /* Search for the previous local maximum... */
          int num_edges = 1;
          int max = PREV_INDEX(min, cnt_vertices);

          while (NOT_RMAX(edge_table, max, cnt_vertices)) {
            num_edges++;
            max = PREV_INDEX(max, cnt_vertices);
          }

          /* Build the previous edge list */
          edge_table[e_index].bstate[BELOW] = bundle_state::UNBUNDLED;
          edge_table[e_index].bundle[BELOW][CLIP] = FALSE;
          edge_table[e_index].bundle[BELOW][SUBJ] = FALSE;

          int v = min;
          for (int i = 0; i < num_edges; i++) {
            edge_table[e_index + i].xb = edge_table[v].vertex.x;
            edge_table[e_index + i].bot.x = edge_table[v].vertex.x;
            edge_table[e_index + i].bot.y = edge_table[v].vertex.y;

            v = PREV_INDEX(v, cnt_vertices);

            edge_table[e_index + i].top.x = edge_table[v].vertex.x;
            edge_table[e_index + i].top.y = edge_table[v].vertex.y;
            edge_table[e_index + i].dx =
                (edge_table[v].vertex.x - edge_table[e_index + i].bot.x) /
                (edge_table[e_index + i].top.y - edge_table[e_index + i].bot.y);
            edge_table[e_index + i].type = type;
            edge_table[e_index + i].outp[ABOVE] = nullptr;
            edge_table[e_index + i].outp[BELOW] = nullptr;
            edge_table[e_index + i].next = nullptr;
            edge_table[e_index + i].prev = nullptr;
            edge_table[e_index + i].succ =
                ((num_edges > 1) && (i < (num_edges - 1)))
                    ? &(edge_table[e_index + i + 1])
                    : nullptr;
            edge_table[e_index + i].pred = ((num_edges > 1) && (i > 0))
                                               ? &(edge_table[e_index + i - 1])
                                               : nullptr;
            edge_table[e_index + i].next_bound = nullptr;
            edge_table[e_index + i].bside[CLIP] =
                (op == gpc_op::GPC_DIFF) ? RIGHT : LEFT;
            edge_table[e_index + i].bside[SUBJ] = LEFT;
          }

          // edge_node *e = &edge_table[e_index];
          insert_bound(bound_list(edge_table[min].vertex.y),
                       &edge_table[e_index]);

          e_index += num_edges;
        }
      }
    }
  }

  edge_tables.push_back(edge_table);
}

gpc::edge_node **gpc::Lmt::bound_list(double y) {
  if (lmt_list.empty()) {
    /* Add node onto the tail end of the LMT */
    lmt_list.push_back(lmt_node(y, nullptr));
    return &(lmt_list.back().second);
  }

  for (auto it = lmt_list.begin(); it != lmt_list.end(); ++it) {
    if (y < it->first) {
      /* Insert a new LMT node before the current node */
      lmt_list.insert(it, lmt_node(y, nullptr));
      return &(prev(it)->second);
    } else if (y == it->first) {
      return &(it->second);
    }
  }

  /* Add node onto the tail end of the LMT */
  lmt_list.push_back(lmt_node(y, nullptr));
  return &(lmt_list.back().second);
}