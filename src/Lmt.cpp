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
  for (auto it = edge_tables.begin(); it != edge_tables.end(); ++it) {
    delete[] * it;
  }

  edge_tables.clear();
  lmt_list.clear();
  sbtree.clear();
}

gpc::edge_node *gpc::Lmt::build_lmt(gpc_polygon *p, int type, gpc_op op) {
  int min, max, num_edges, v;
  int e_index = 0;
  edge_node *e;

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
  MALLOC(edge_table, total_vertices * sizeof(edge_node), "edge table creation",
         edge_node);

  for (int c = 0; c < p->num_contours(); ++c) {
    if (p->contour[c].is_contributing) {
      /* Perform contour optimisation */
      int cnt_vertices = 0;

      for (int i = 0; i < p->contour[c].num_vertices(); ++i)
        if (optimal(p->contour[c].vertex, i, p->contour[c].num_vertices())) {
          edge_table[cnt_vertices].vertex.x = p->contour[c].vertex[i].x;
          edge_table[cnt_vertices].vertex.y = p->contour[c].vertex[i].y;

          /* Record vertex in the scanbeam table */
          sbtree.push_back(p->contour[c].vertex[i].y);

          ++cnt_vertices;
        }

      /* Do the contour forward pass */
      for (int min = 0; min < cnt_vertices; min++) {
        /* If a forward local minimum... */
        if (FWD_MIN(edge_table, min, cnt_vertices)) {
          /* Search for the next local maximum... */
          num_edges = 1;
          max = NEXT_INDEX(min, cnt_vertices);
          while (NOT_FMAX(edge_table, max, cnt_vertices)) {
            num_edges++;
            max = NEXT_INDEX(max, cnt_vertices);
          }

          /* Build the next edge list */
          e = &edge_table[e_index];
          e_index += num_edges;
          v = min;
          e[0].bstate[BELOW] = bundle_state::UNBUNDLED;
          e[0].bundle[BELOW][CLIP] = FALSE;
          e[0].bundle[BELOW][SUBJ] = FALSE;
          for (int i = 0; i < num_edges; ++i) {
            e[i].xb = edge_table[v].vertex.x;
            e[i].bot.x = edge_table[v].vertex.x;
            e[i].bot.y = edge_table[v].vertex.y;

            v = NEXT_INDEX(v, cnt_vertices);

            e[i].top.x = edge_table[v].vertex.x;
            e[i].top.y = edge_table[v].vertex.y;
            e[i].dx = (edge_table[v].vertex.x - e[i].bot.x) /
                      (e[i].top.y - e[i].bot.y);
            e[i].type = type;
            e[i].outp[ABOVE] = NULL;
            e[i].outp[BELOW] = NULL;
            e[i].next = NULL;
            e[i].prev = NULL;
            e[i].succ =
                ((num_edges > 1) && (i < (num_edges - 1))) ? &(e[i + 1]) : NULL;
            e[i].pred = ((num_edges > 1) && (i > 0)) ? &(e[i - 1]) : NULL;
            e[i].next_bound = NULL;
            e[i].bside[CLIP] = (op == gpc_op::GPC_DIFF) ? RIGHT : LEFT;
            e[i].bside[SUBJ] = LEFT;
          }
          insert_bound(bound_list(edge_table[min].vertex.y), e);
        }
      }

      /* Do the contour reverse pass */
      for (min = 0; min < cnt_vertices; min++) {
        /* If a reverse local minimum... */
        if (REV_MIN(edge_table, min, cnt_vertices)) {
          /* Search for the previous local maximum... */
          num_edges = 1;
          max = PREV_INDEX(min, cnt_vertices);
          while (NOT_RMAX(edge_table, max, cnt_vertices)) {
            num_edges++;
            max = PREV_INDEX(max, cnt_vertices);
          }

          /* Build the previous edge list */
          e = &edge_table[e_index];
          e_index += num_edges;
          v = min;
          e[0].bstate[BELOW] = bundle_state::UNBUNDLED;
          e[0].bundle[BELOW][CLIP] = FALSE;
          e[0].bundle[BELOW][SUBJ] = FALSE;
          for (int i = 0; i < num_edges; i++) {
            e[i].xb = edge_table[v].vertex.x;
            e[i].bot.x = edge_table[v].vertex.x;
            e[i].bot.y = edge_table[v].vertex.y;

            v = PREV_INDEX(v, cnt_vertices);

            e[i].top.x = edge_table[v].vertex.x;
            e[i].top.y = edge_table[v].vertex.y;
            e[i].dx = (edge_table[v].vertex.x - e[i].bot.x) /
                      (e[i].top.y - e[i].bot.y);
            e[i].type = type;
            e[i].outp[ABOVE] = NULL;
            e[i].outp[BELOW] = NULL;
            e[i].next = NULL;
            e[i].prev = NULL;
            e[i].succ =
                ((num_edges > 1) && (i < (num_edges - 1))) ? &(e[i + 1]) : NULL;
            e[i].pred = ((num_edges > 1) && (i > 0)) ? &(e[i - 1]) : NULL;
            e[i].next_bound = NULL;
            e[i].bside[CLIP] = (op == gpc_op::GPC_DIFF) ? RIGHT : LEFT;
            e[i].bside[SUBJ] = LEFT;
          }
          insert_bound(bound_list(edge_table[min].vertex.y), e);
        }
      }
    }
  }
  return edge_table;
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