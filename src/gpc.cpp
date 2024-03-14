#include "gpc.hpp"

namespace gpc {

typedef struct it_shape /* Intersection table                */
{
  edge_node *ie[2];                /* Intersecting edge (bundle) pair   */
  gpc_vertex point;                /* Point of intersection             */
  struct it_shape *next = nullptr; /* The next intersection table node  */
} it_node;

typedef struct st_shape /* Sorted edge table                 */
{
  edge_node *edge;       /* Pointer to AET edge               */
  double xb;             /* Scanbeam bottom x coordinate      */
  double xt;             /* Scanbeam top x coordinate         */
  double dx;             /* Change in x for a unit y increase */
  struct st_shape *prev; /* Previous edge in sorted list      */
} st_node;

/*
===========================================================================
                             Private Functions
===========================================================================
*/

static void reset_it(it_node **it) {
  it_node *itn;

  while (*it) {
    itn = (*it)->next;
    delete *it;
    *it = itn;
  }
}

static void add_edge_to_aet(edge_node **aet, edge_node *edge, edge_node *prev) {
  if (!*aet) {
    /* Append edge onto the tail end of the AET */
    *aet = edge;
    edge->prev = prev;
    edge->next = nullptr;
  } else {
    /* Do primary sort on the xb field */
    if (edge->xb < (*aet)->xb) {
      /* Insert edge here (before the AET edge) */
      edge->prev = prev;
      edge->next = *aet;
      (*aet)->prev = edge;
      *aet = edge;
    } else {
      if (edge->xb == (*aet)->xb) {
        /* Do secondary sort on the dx field */
        if (edge->dx < (*aet)->dx) {
          /* Insert edge here (before the AET edge) */
          edge->prev = prev;
          edge->next = *aet;
          (*aet)->prev = edge;
          *aet = edge;
        } else {
          /* Head further into the AET */
          add_edge_to_aet(&((*aet)->next), edge, *aet);
        }
      } else {
        /* Head further into the AET */
        add_edge_to_aet(&((*aet)->next), edge, *aet);
      }
    }
  }
}

static void add_intersection(it_node **it, edge_node *edge0, edge_node *edge1,
                             double x, double y) {
  it_node *existing_node;

  if (!*it) {
    /* Append a new node to the tail of the list */
    MALLOC(*it, sizeof(it_node), "IT insertion", it_node);
    (*it)->ie[0] = edge0;
    (*it)->ie[1] = edge1;
    (*it)->point.x = x;
    (*it)->point.y = y;
    (*it)->next = nullptr;
  } else {
    if ((*it)->point.y > y) {
      /* Insert a new node mid-list */
      existing_node = *it;
      MALLOC(*it, sizeof(it_node), "IT insertion", it_node);
      (*it)->ie[0] = edge0;
      (*it)->ie[1] = edge1;
      (*it)->point.x = x;
      (*it)->point.y = y;
      (*it)->next = existing_node;
    } else
      /* Head further down the list */
      add_intersection(&((*it)->next), edge0, edge1, x, y);
  }
}

static void add_st_edge(st_node **st, it_node **it, edge_node *edge,
                        double dy) {
  st_node *existing_node;
  double den, r, x, y;

  if (!*st) {
    /* Append edge onto the tail end of the ST */
    MALLOC(*st, sizeof(st_node), "ST insertion", st_node);
    (*st)->edge = edge;
    (*st)->xb = edge->xb;
    (*st)->xt = edge->xt;
    (*st)->dx = edge->dx;
    (*st)->prev = nullptr;
  } else {
    den = ((*st)->xt - (*st)->xb) - (edge->xt - edge->xb);

    /* If new edge and ST edge don't cross */
    if ((edge->xt >= (*st)->xt) || (edge->dx == (*st)->dx) ||
        (fabs(den) <= DBL_EPSILON)) {
      /* No intersection - insert edge here (before the ST edge) */
      existing_node = *st;
      MALLOC(*st, sizeof(st_node), "ST insertion", st_node);
      (*st)->edge = edge;
      (*st)->xb = edge->xb;
      (*st)->xt = edge->xt;
      (*st)->dx = edge->dx;
      (*st)->prev = existing_node;
    } else {
      /* Compute intersection between new edge and ST edge */
      r = (edge->xb - (*st)->xb) / den;
      x = (*st)->xb + r * ((*st)->xt - (*st)->xb);
      y = r * dy;

      /* Insert the edge pointers and the intersection point in the IT */
      add_intersection(it, (*st)->edge, edge, x, y);

      /* Head further into the ST */
      add_st_edge(&((*st)->prev), it, edge, dy);
    }
  }
}

static void build_intersection_table(it_node **it, edge_node *aet, double dy) {
  st_node *st, *stp;
  edge_node *edge;

  /* Build intersection table for the current scanbeam */
  reset_it(it);
  st = nullptr;

  /* Process each AET edge */
  for (edge = aet; edge; edge = edge->next) {
    if ((edge->bstate[ABOVE] == bundle_state::BUNDLE_HEAD) ||
        edge->bundle[ABOVE][CLIP] || edge->bundle[ABOVE][SUBJ])
      add_st_edge(&st, it, edge, dy);
  }

  /* Free the sorted edge table */
  while (st) {
    stp = st->prev;
    delete (st);
    st = stp;
  }
}

static void swap_intersecting_edge_bundles(edge_node **aet,
                                           it_node *intersect) {
  edge_node *e0 = intersect->ie[0];
  edge_node *e1 = intersect->ie[1];
  edge_node *e0t = e0;
  edge_node *e1t = e1;
  edge_node *e0n = e0->next;
  edge_node *e1n = e1->next;

  // Find the node before the e0 bundle
  edge_node *e0p = e0->prev;
  if (e0->bstate[ABOVE] == bundle_state::BUNDLE_HEAD) {
    do {
      e0t = e0p;
      e0p = e0p->prev;
    } while (e0p && (e0p->bstate[ABOVE] == bundle_state::BUNDLE_TAIL));
  }

  // Find the node before the e1 bundle
  edge_node *e1p = e1->prev;
  if (e1->bstate[ABOVE] == bundle_state::BUNDLE_HEAD) {
    do {
      e1t = e1p;
      e1p = e1p->prev;
    } while (e1p && (e1p->bstate[ABOVE] == bundle_state::BUNDLE_TAIL));
  }

  // Swap the e0p and e1p links
  if (e0p) {
    if (e1p) {
      if (e0p != e1) {
        e0p->next = e1t;
        e1t->prev = e0p;
      }
      if (e1p != e0) {
        e1p->next = e0t;
        e0t->prev = e1p;
      }
    } else {
      if (e0p != e1) {
        e0p->next = e1t;
        e1t->prev = e0p;
      }
      *aet = e0t;
      e0t->prev = nullptr;
    }
  } else {
    if (e1p != e0) {
      e1p->next = e0t;
      e0t->prev = e1p;
    }
    *aet = e1t;
    e1t->prev = nullptr;
  }

  // Re-link after e0
  if (e0p != e1) {
    e0->next = e1n;
    if (e1n) {
      e1n->prev = e0;
    }
  } else {
    e0->next = e1t;
    e1t->prev = e0;
  }

  // Re-link after e1
  if (e1p != e0) {
    e1->next = e0n;
    if (e0n) {
      e0n->prev = e1;
    }
  } else {
    e1->next = e0t;
    e0t->prev = e1;
  }
}

static int count_contours(polygon_node *polygon) {
  int nc, nv;
  vertex_node *v, *nextv;

  for (nc = 0; polygon; polygon = polygon->next)
    if (polygon->active) {
      /* Count the vertices in the current contour */
      nv = 0;
      for (v = polygon->proxy->v[LEFT]; v; v = v->next)
        nv++;

      /* Record valid vertex counts in the active field */
      if (nv > 2) {
        polygon->active = nv;
        nc++;
      } else {
        /* Invalid contour: just free the heap */
        for (v = polygon->proxy->v[LEFT]; v; v = nextv) {
          nextv = v->next;
          delete v;
        }
        polygon->active = 0;
      }
    }
  return nc;
}

static void add_left(polygon_node *p, double x, double y) {
  vertex_node *nv;

  /* Create a new vertex node and set its fields */
  MALLOC(nv, sizeof(vertex_node), "vertex node creation", vertex_node);
  nv->x = x;
  nv->y = y;

  /* Add vertex nv to the left end of the polygon's vertex list */
  nv->next = p->proxy->v[LEFT];

  /* Update proxy->[LEFT] to point to nv */
  p->proxy->v[LEFT] = nv;
}

static void merge_left(polygon_node *p, polygon_node *q, polygon_node *list) {
  polygon_node *target;

  /* Label contour as a hole */
  q->proxy->hole = TRUE;

  if (p->proxy != q->proxy) {
    /* Assign p's vertex list to the left end of q's list */
    p->proxy->v[RIGHT]->next = q->proxy->v[LEFT];
    q->proxy->v[LEFT] = p->proxy->v[LEFT];

    /* Redirect any p->proxy references to q->proxy */

    for (target = p->proxy; list; list = list->next) {
      if (list->proxy == target) {
        list->active = FALSE;
        list->proxy = q->proxy;
      }
    }
  }
}

static void add_right(polygon_node *p, double x, double y) {
  vertex_node *nv;

  /* Create a new vertex node and set its fields */
  MALLOC(nv, sizeof(vertex_node), "vertex node creation", vertex_node);
  nv->x = x;
  nv->y = y;
  nv->next = nullptr;

  /* Add vertex nv to the right end of the polygon's vertex list */
  p->proxy->v[RIGHT]->next = nv;

  /* Update proxy->v[RIGHT] to point to nv */
  p->proxy->v[RIGHT] = nv;
}

static void merge_right(polygon_node *p, polygon_node *q, polygon_node *list) {
  polygon_node *target;

  /* Label contour as external */
  q->proxy->hole = FALSE;

  if (p->proxy != q->proxy) {
    /* Assign p's vertex list to the right end of q's list */
    q->proxy->v[RIGHT]->next = p->proxy->v[LEFT];
    q->proxy->v[RIGHT] = p->proxy->v[RIGHT];

    /* Redirect any p->proxy references to q->proxy */
    for (target = p->proxy; list; list = list->next) {
      if (list->proxy == target) {
        list->active = FALSE;
        list->proxy = q->proxy;
      }
    }
  }
}

static void add_local_min(polygon_node **p, edge_node *edge, double x,
                          double y) {
  polygon_node *existing_min;
  vertex_node *nv;

  existing_min = *p;

  MALLOC(*p, sizeof(polygon_node), "polygon node creation", polygon_node);

  /* Create a new vertex node and set its fields */
  MALLOC(nv, sizeof(vertex_node), "vertex node creation", vertex_node);
  nv->x = x;
  nv->y = y;
  nv->next = nullptr;

  /* Initialise proxy to point to p itself */
  (*p)->proxy = (*p);
  (*p)->active = TRUE;
  (*p)->next = existing_min;

  /* Make v[LEFT] and v[RIGHT] point to new vertex nv */
  (*p)->v[LEFT] = nv;
  (*p)->v[RIGHT] = nv;

  /* Assign polygon p to the edge */
  edge->outp[ABOVE] = *p;
}

static int count_tristrips(polygon_node *tn) {
  int total;

  for (total = 0; tn; tn = tn->next)
    if (tn->active > 2)
      total++;
  return total;
}

static void add_vertex(vertex_node **t, double x, double y) {
  if (!(*t)) {
    MALLOC(*t, sizeof(vertex_node), "tristrip vertex creation", vertex_node);
    (*t)->x = x;
    (*t)->y = y;
    (*t)->next = nullptr;
  } else
    /* Head further down the list */
    add_vertex(&((*t)->next), x, y);
}

static void new_tristrip(polygon_node **tn, edge_node *edge, double x,
                         double y) {
  if (!(*tn)) {
    MALLOC(*tn, sizeof(polygon_node), "tristrip node creation", polygon_node);
    (*tn)->next = nullptr;
    (*tn)->v[LEFT] = nullptr;
    (*tn)->v[RIGHT] = nullptr;
    (*tn)->active = 1;
    add_vertex(&((*tn)->v[LEFT]), x, y);
    edge->outp[ABOVE] = *tn;
  } else
    /* Head further down the list */
    new_tristrip(&((*tn)->next), edge, x, y);
}

/*
===========================================================================
                             Public Functions
===========================================================================
*/

void gpc_polygon_clip(gpc_op op, gpc_polygon *subj, gpc_polygon *clip,
                      gpc_polygon *result) {
  // clear result
  result->contour.clear();

  if (subj->num_contours() == 0 && clip->num_contours() == 0) {
    return;
  }

  if (clip->num_contours() == 0) {
    if (op == gpc_op::GPC_INT) {
      return;
    } else {
      *result = *subj;
      return;
    }
  }

  if (subj->num_contours() == 0) {
    if ((op == gpc_op::GPC_INT) || (op == gpc_op::GPC_DIFF)) {
      return;
    } else {
      *result = *clip;
      return;
    }
  }

  // subj 和 clip 都不为空
  /* 确定可能有贡献的轮廓 */
  if ((op == gpc_op::GPC_INT) || (op == gpc_op::GPC_DIFF)) {
    minimax_test(subj, clip, op);
  }

  /* 构建局部最小表 */
  Lmt lmt;
  lmt.build_lmt(subj, SUBJ, op);
  lmt.build_lmt(clip, CLIP, op);

  std::sort(lmt.sbtree.begin(), lmt.sbtree.end());
  const std::vector<double> &sbt = lmt.sbtree;

  /* 如果没有轮廓有贡献，返回空结果 */
  if (lmt.lmt_list.empty()) {
    return;
  }

  /* 对于差集操作，反转剪切多边形 */
  int parity[2] = {LEFT, LEFT};
  if (op == gpc_op::GPC_DIFF)
    parity[CLIP] = RIGHT;

  it_node *it = nullptr, *intersect;
  edge_node *edge, *prev_edge, *next_edge, *succ_edge, *e0, *e1;
  edge_node *aet = nullptr;
  polygon_node *out_poly = nullptr, *p, *q, *poly, *npoly, *cf = nullptr;
  vertex_node *vtx, *nv;
  h_state horiz[2];
  int in[2], exists[2];
  int c, v, contributing;
  int vclass, bl, br, tl, tr;
  double xb, px, ix, iy;

  /* 处理每个扫描线 */
  int scanbeam = 0;
  double yb, yt, dy;
  while (scanbeam < sbt.size()) {
    /* Set yb and yt to the bottom and top of the scanbeam */
    yb = sbt[scanbeam++];

    while (scanbeam < sbt.size()) {
      if (sbt[scanbeam] == yb)
        ++scanbeam;
      else
        break;
    }

    yt = sbt[scanbeam];
    dy = yt - yb;

    /* === SCANBEAM BOUNDARY PROCESSING ================================ */

    /* If LMT node corresponding to yb exists */
    if (!lmt.lmt_list.empty()) {
      if (lmt.lmt_list.front().first == yb) {
        /* Add edges starting at this local minimum to the AET */
        for (auto edge = lmt.lmt_list.front().second; edge;
             edge = edge->next_bound) {
          add_edge_to_aet(&aet, edge, nullptr);
        }

        lmt.lmt_list.pop_front();
      }
    }

    /* Set dummy previous x value */
    px = -DBL_MAX;

    /* Create bundles within AET */
    e0 = aet;
    e1 = aet;

    /* Set up bundle fields of first edge */
    aet->bundle[ABOVE][aet->type] = (aet->top.y != yb);
    aet->bundle[ABOVE][!aet->type] = FALSE;
    aet->bstate[ABOVE] = bundle_state::UNBUNDLED;

    for (next_edge = aet->next; next_edge; next_edge = next_edge->next) {
      /* Set up bundle fields of next edge */
      next_edge->bundle[ABOVE][next_edge->type] = (next_edge->top.y != yb);
      next_edge->bundle[ABOVE][!next_edge->type] = FALSE;
      next_edge->bstate[ABOVE] = bundle_state::UNBUNDLED;

      /* Bundle edges above the scanbeam boundary if they coincide */
      if (next_edge->bundle[ABOVE][next_edge->type]) {
        if (EQ(e0->xb, next_edge->xb) && EQ(e0->dx, next_edge->dx) &&
            (e0->top.y != yb)) {
          next_edge->bundle[ABOVE][next_edge->type] ^=
              e0->bundle[ABOVE][next_edge->type];
          next_edge->bundle[ABOVE][!next_edge->type] =
              e0->bundle[ABOVE][!next_edge->type];
          next_edge->bstate[ABOVE] = bundle_state::BUNDLE_HEAD;
          e0->bundle[ABOVE][CLIP] = FALSE;
          e0->bundle[ABOVE][SUBJ] = FALSE;
          e0->bstate[ABOVE] = bundle_state::BUNDLE_TAIL;
        }
        e0 = next_edge;
      }
    }

    horiz[CLIP] = h_state::NH;
    horiz[SUBJ] = h_state::NH;

    /* Process each edge at this scanbeam boundary */
    for (edge = aet; edge; edge = edge->next) {
      exists[CLIP] =
          edge->bundle[ABOVE][CLIP] + (edge->bundle[BELOW][CLIP] << 1);
      exists[SUBJ] =
          edge->bundle[ABOVE][SUBJ] + (edge->bundle[BELOW][SUBJ] << 1);

      if (exists[CLIP] || exists[SUBJ]) {
        /* Set bundle side */
        edge->bside[CLIP] = parity[CLIP];
        edge->bside[SUBJ] = parity[SUBJ];

        /* Determine contributing status and quadrant occupancies */
        switch (op) {
        case gpc_op::GPC_DIFF:
        case gpc_op::GPC_INT:
          contributing =
              (exists[CLIP] && (parity[SUBJ] || horiz[SUBJ])) ||
              (exists[SUBJ] && (parity[CLIP] || horiz[CLIP])) ||
              (exists[CLIP] && exists[SUBJ] && (parity[CLIP] == parity[SUBJ]));
          br = (parity[CLIP]) && (parity[SUBJ]);
          bl = (parity[CLIP] ^ edge->bundle[ABOVE][CLIP]) &&
               (parity[SUBJ] ^ edge->bundle[ABOVE][SUBJ]);
          tr = (parity[CLIP] ^ (horiz[CLIP] != h_state::NH)) &&
               (parity[SUBJ] ^ (horiz[SUBJ] != h_state::NH));
          tl = (parity[CLIP] ^ (horiz[CLIP] != h_state::NH) ^
                edge->bundle[BELOW][CLIP]) &&
               (parity[SUBJ] ^ (horiz[SUBJ] != h_state::NH) ^
                edge->bundle[BELOW][SUBJ]);
          break;
        case gpc_op::GPC_XOR:
          contributing = exists[CLIP] || exists[SUBJ];
          br = (parity[CLIP]) ^ (parity[SUBJ]);
          bl = (parity[CLIP] ^ edge->bundle[ABOVE][CLIP]) ^
               (parity[SUBJ] ^ edge->bundle[ABOVE][SUBJ]);
          tr = (parity[CLIP] ^ (horiz[CLIP] != h_state::NH)) ^
               (parity[SUBJ] ^ (horiz[SUBJ] != h_state::NH));
          tl = (parity[CLIP] ^ (horiz[CLIP] != h_state::NH) ^
                edge->bundle[BELOW][CLIP]) ^
               (parity[SUBJ] ^ (horiz[SUBJ] != h_state::NH) ^
                edge->bundle[BELOW][SUBJ]);
          break;
        case gpc_op::GPC_UNION:
          contributing =
              (exists[CLIP] && (!parity[SUBJ] || horiz[SUBJ])) ||
              (exists[SUBJ] && (!parity[CLIP] || horiz[CLIP])) ||
              (exists[CLIP] && exists[SUBJ] && (parity[CLIP] == parity[SUBJ]));
          br = (parity[CLIP]) || (parity[SUBJ]);
          bl = (parity[CLIP] ^ edge->bundle[ABOVE][CLIP]) ||
               (parity[SUBJ] ^ edge->bundle[ABOVE][SUBJ]);
          tr = (parity[CLIP] ^ (horiz[CLIP] != h_state::NH)) ||
               (parity[SUBJ] ^ (horiz[SUBJ] != h_state::NH));
          tl = (parity[CLIP] ^ (horiz[CLIP] != h_state::NH) ^
                edge->bundle[BELOW][CLIP]) ||
               (parity[SUBJ] ^ (horiz[SUBJ] != h_state::NH) ^
                edge->bundle[BELOW][SUBJ]);
          break;
        }

        /* Update parity */
        parity[CLIP] ^= edge->bundle[ABOVE][CLIP];
        parity[SUBJ] ^= edge->bundle[ABOVE][SUBJ];

        /* Update horizontal state */
        if (exists[CLIP])
          horiz[CLIP] = next_h_state[horiz[CLIP]]
                                    [((exists[CLIP] - 1) << 1) + parity[CLIP]];
        if (exists[SUBJ])
          horiz[SUBJ] = next_h_state[horiz[SUBJ]]
                                    [((exists[SUBJ] - 1) << 1) + parity[SUBJ]];

        vclass = tr + (tl << 1) + (br << 2) + (bl << 3);

        if (contributing) {
          xb = edge->xb;

          switch (vclass) {
          case vertex_type::EMN:
          case vertex_type::IMN:
            add_local_min(&out_poly, edge, xb, yb);
            px = xb;
            cf = edge->outp[ABOVE];
            break;
          case vertex_type::ERI:
            if (xb != px) {
              add_right(cf, xb, yb);
              px = xb;
            }
            edge->outp[ABOVE] = cf;
            cf = nullptr;
            break;
          case vertex_type::ELI:
            add_left(edge->outp[BELOW], xb, yb);
            px = xb;
            cf = edge->outp[BELOW];
            break;
          case vertex_type::EMX:
            if (xb != px) {
              add_left(cf, xb, yb);
              px = xb;
            }
            merge_right(cf, edge->outp[BELOW], out_poly);
            cf = nullptr;
            break;
          case vertex_type::ILI:
            if (xb != px) {
              add_left(cf, xb, yb);
              px = xb;
            }
            edge->outp[ABOVE] = cf;
            cf = nullptr;
            break;
          case vertex_type::IRI:
            add_right(edge->outp[BELOW], xb, yb);
            px = xb;
            cf = edge->outp[BELOW];
            edge->outp[BELOW] = nullptr;
            break;
          case vertex_type::IMX:
            if (xb != px) {
              add_right(cf, xb, yb);
              px = xb;
            }
            merge_left(cf, edge->outp[BELOW], out_poly);
            cf = nullptr;
            edge->outp[BELOW] = nullptr;
            break;
          case vertex_type::IMM:
            if (xb != px) {
              add_right(cf, xb, yb);
              px = xb;
            }
            merge_left(cf, edge->outp[BELOW], out_poly);
            edge->outp[BELOW] = nullptr;
            add_local_min(&out_poly, edge, xb, yb);
            cf = edge->outp[ABOVE];
            break;
          case vertex_type::EMM:
            if (xb != px) {
              add_left(cf, xb, yb);
              px = xb;
            }
            merge_right(cf, edge->outp[BELOW], out_poly);
            edge->outp[BELOW] = nullptr;
            add_local_min(&out_poly, edge, xb, yb);
            cf = edge->outp[ABOVE];
            break;
          case vertex_type::LED:
            if (edge->bot.y == yb)
              add_left(edge->outp[BELOW], xb, yb);
            edge->outp[ABOVE] = edge->outp[BELOW];
            px = xb;
            break;
          case vertex_type::RED:
            if (edge->bot.y == yb)
              add_right(edge->outp[BELOW], xb, yb);
            edge->outp[ABOVE] = edge->outp[BELOW];
            px = xb;
            break;
          default:
            break;
          } /* End of switch */
        }   /* End of contributing conditional */
      }     /* End of edge exists conditional */
    }       /* End of AET loop */

    /* Delete terminating edges from the AET, otherwise compute xt */
    for (edge = aet; edge; edge = edge->next) {
      if (edge->top.y == yb) {
        prev_edge = edge->prev;
        next_edge = edge->next;
        if (prev_edge)
          prev_edge->next = next_edge;
        else
          aet = next_edge;
        if (next_edge)
          next_edge->prev = prev_edge;

        /* Copy bundle head state to the adjacent tail edge if required */
        if ((edge->bstate[BELOW] == bundle_state::BUNDLE_HEAD) && prev_edge) {
          if (prev_edge->bstate[BELOW] == bundle_state::BUNDLE_TAIL) {
            prev_edge->outp[BELOW] = edge->outp[BELOW];
            prev_edge->bstate[BELOW] = bundle_state::UNBUNDLED;
            if (prev_edge->prev)
              if (prev_edge->prev->bstate[BELOW] == bundle_state::BUNDLE_TAIL)
                prev_edge->bstate[BELOW] = bundle_state::BUNDLE_HEAD;
          }
        }
      } else {
        if (edge->top.y == yt)
          edge->xt = edge->top.x;
        else
          edge->xt = edge->bot.x + edge->dx * (yt - edge->bot.y);
      }
    }

    if (scanbeam < sbt.size()) {
      /* === SCANBEAM INTERIOR PROCESSING ============================== */

      build_intersection_table(&it, aet, dy);

      /* Process each node in the intersection table */
      for (intersect = it; intersect; intersect = intersect->next) {
        e0 = intersect->ie[0];
        e1 = intersect->ie[1];

        /* Only generate output for contributing intersections */
        if ((e0->bundle[ABOVE][CLIP] || e0->bundle[ABOVE][SUBJ]) &&
            (e1->bundle[ABOVE][CLIP] || e1->bundle[ABOVE][SUBJ])) {
          p = e0->outp[ABOVE];
          q = e1->outp[ABOVE];
          ix = intersect->point.x;
          iy = intersect->point.y + yb;

          in[CLIP] = (e0->bundle[ABOVE][CLIP] && !e0->bside[CLIP]) ||
                     (e1->bundle[ABOVE][CLIP] && e1->bside[CLIP]) ||
                     (!e0->bundle[ABOVE][CLIP] && !e1->bundle[ABOVE][CLIP] &&
                      e0->bside[CLIP] && e1->bside[CLIP]);
          in[SUBJ] = (e0->bundle[ABOVE][SUBJ] && !e0->bside[SUBJ]) ||
                     (e1->bundle[ABOVE][SUBJ] && e1->bside[SUBJ]) ||
                     (!e0->bundle[ABOVE][SUBJ] && !e1->bundle[ABOVE][SUBJ] &&
                      e0->bside[SUBJ] && e1->bside[SUBJ]);

          /* Determine quadrant occupancies */
          switch (op) {
          case gpc_op::GPC_DIFF:
          case gpc_op::GPC_INT:
            tr = (in[CLIP]) && (in[SUBJ]);
            tl = (in[CLIP] ^ e1->bundle[ABOVE][CLIP]) &&
                 (in[SUBJ] ^ e1->bundle[ABOVE][SUBJ]);
            br = (in[CLIP] ^ e0->bundle[ABOVE][CLIP]) &&
                 (in[SUBJ] ^ e0->bundle[ABOVE][SUBJ]);
            bl = (in[CLIP] ^ e1->bundle[ABOVE][CLIP] ^
                  e0->bundle[ABOVE][CLIP]) &&
                 (in[SUBJ] ^ e1->bundle[ABOVE][SUBJ] ^ e0->bundle[ABOVE][SUBJ]);
            break;
          case gpc_op::GPC_XOR:
            tr = (in[CLIP]) ^ (in[SUBJ]);
            tl = (in[CLIP] ^ e1->bundle[ABOVE][CLIP]) ^
                 (in[SUBJ] ^ e1->bundle[ABOVE][SUBJ]);
            br = (in[CLIP] ^ e0->bundle[ABOVE][CLIP]) ^
                 (in[SUBJ] ^ e0->bundle[ABOVE][SUBJ]);
            bl =
                (in[CLIP] ^ e1->bundle[ABOVE][CLIP] ^ e0->bundle[ABOVE][CLIP]) ^
                (in[SUBJ] ^ e1->bundle[ABOVE][SUBJ] ^ e0->bundle[ABOVE][SUBJ]);
            break;
          case gpc_op::GPC_UNION:
            tr = (in[CLIP]) || (in[SUBJ]);
            tl = (in[CLIP] ^ e1->bundle[ABOVE][CLIP]) ||
                 (in[SUBJ] ^ e1->bundle[ABOVE][SUBJ]);
            br = (in[CLIP] ^ e0->bundle[ABOVE][CLIP]) ||
                 (in[SUBJ] ^ e0->bundle[ABOVE][SUBJ]);
            bl = (in[CLIP] ^ e1->bundle[ABOVE][CLIP] ^
                  e0->bundle[ABOVE][CLIP]) ||
                 (in[SUBJ] ^ e1->bundle[ABOVE][SUBJ] ^ e0->bundle[ABOVE][SUBJ]);
            break;
          }

          vclass = tr + (tl << 1) + (br << 2) + (bl << 3);

          switch (vclass) {
          case vertex_type::EMN:
            add_local_min(&out_poly, e0, ix, iy);
            e1->outp[ABOVE] = e0->outp[ABOVE];
            break;
          case vertex_type::ERI:
            if (p) {
              add_right(p, ix, iy);
              e1->outp[ABOVE] = p;
              e0->outp[ABOVE] = nullptr;
            }
            break;
          case vertex_type::ELI:
            if (q) {
              add_left(q, ix, iy);
              e0->outp[ABOVE] = q;
              e1->outp[ABOVE] = nullptr;
            }
            break;
          case vertex_type::EMX:
            if (p && q) {
              add_left(p, ix, iy);
              merge_right(p, q, out_poly);
              e0->outp[ABOVE] = nullptr;
              e1->outp[ABOVE] = nullptr;
            }
            break;
          case vertex_type::IMN:
            add_local_min(&out_poly, e0, ix, iy);
            e1->outp[ABOVE] = e0->outp[ABOVE];
            break;
          case vertex_type::ILI:
            if (p) {
              add_left(p, ix, iy);
              e1->outp[ABOVE] = p;
              e0->outp[ABOVE] = nullptr;
            }
            break;
          case vertex_type::IRI:
            if (q) {
              add_right(q, ix, iy);
              e0->outp[ABOVE] = q;
              e1->outp[ABOVE] = nullptr;
            }
            break;
          case vertex_type::IMX:
            if (p && q) {
              add_right(p, ix, iy);
              merge_left(p, q, out_poly);
              e0->outp[ABOVE] = nullptr;
              e1->outp[ABOVE] = nullptr;
            }
            break;
          case vertex_type::IMM:
            if (p && q) {
              add_right(p, ix, iy);
              merge_left(p, q, out_poly);
              add_local_min(&out_poly, e0, ix, iy);
              e1->outp[ABOVE] = e0->outp[ABOVE];
            }
            break;
          case vertex_type::EMM:
            if (p && q) {
              add_left(p, ix, iy);
              merge_right(p, q, out_poly);
              add_local_min(&out_poly, e0, ix, iy);
              e1->outp[ABOVE] = e0->outp[ABOVE];
            }
            break;
          default:
            break;
          } /* End of switch */
        }   /* End of contributing intersection conditional */

        /* Swap bundle sides in response to edge crossing */
        if (e0->bundle[ABOVE][CLIP])
          e1->bside[CLIP] = !e1->bside[CLIP];
        if (e1->bundle[ABOVE][CLIP])
          e0->bside[CLIP] = !e0->bside[CLIP];
        if (e0->bundle[ABOVE][SUBJ])
          e1->bside[SUBJ] = !e1->bside[SUBJ];
        if (e1->bundle[ABOVE][SUBJ])
          e0->bside[SUBJ] = !e0->bside[SUBJ];

        /* Swap the edge bundles in the aet */
        swap_intersecting_edge_bundles(&aet, intersect);

      } /* End of IT loop*/

      /* Prepare for next scanbeam */
      for (edge = aet; edge; edge = next_edge) {
        next_edge = edge->next;
        succ_edge = edge->succ;

        if ((edge->top.y == yt) && succ_edge) {
          /* Replace AET edge by its successor */
          succ_edge->outp[BELOW] = edge->outp[ABOVE];
          succ_edge->bstate[BELOW] = edge->bstate[ABOVE];
          succ_edge->bundle[BELOW][CLIP] = edge->bundle[ABOVE][CLIP];
          succ_edge->bundle[BELOW][SUBJ] = edge->bundle[ABOVE][SUBJ];
          prev_edge = edge->prev;
          if (prev_edge)
            prev_edge->next = succ_edge;
          else
            aet = succ_edge;
          if (next_edge)
            next_edge->prev = succ_edge;
          succ_edge->prev = prev_edge;
          succ_edge->next = next_edge;
        } else {
          /* Update this edge */
          edge->outp[BELOW] = edge->outp[ABOVE];
          edge->bstate[BELOW] = edge->bstate[ABOVE];
          edge->bundle[BELOW][CLIP] = edge->bundle[ABOVE][CLIP];
          edge->bundle[BELOW][SUBJ] = edge->bundle[ABOVE][SUBJ];
          edge->xb = edge->xt;
        }
        edge->outp[ABOVE] = nullptr;
      }
    }
  } /* === END OF SCANBEAM PROCESSING ================================== */

  /* Generate result polygon from out_poly */
  if (count_contours(out_poly) > 0) {
    result->hole.resize(count_contours(out_poly));
    result->contour.resize(count_contours(out_poly));

    c = 0;
    for (poly = out_poly; poly; poly = npoly) {
      npoly = poly->next;
      if (poly->active) {
        result->hole[c] = poly->proxy->hole;

        result->contour[c].vertex.resize(poly->active);

        v = result->contour[c].vertex.size() - 1;
        for (vtx = poly->proxy->v[LEFT]; vtx; vtx = nv) {
          nv = vtx->next;
          result->contour[c].vertex[v].x = vtx->x;
          result->contour[c].vertex[v].y = vtx->y;
          delete (vtx);
          v--;
        }
        c++;
      }
      delete poly;
    }
  } else {
    for (poly = out_poly; poly; poly = npoly) {
      npoly = poly->next;
      delete (poly);
    }
  }

  /* Tidy up */
  reset_it(&it);
}

void gpc_free_tristrip(gpc_tristrip *t) {
  for (int s = 0; s < t->num_strips; ++s)
    t->strip[s].vertex.clear();

  delete (t->strip);
  t->num_strips = 0;
}

void gpc_tristrip_clip(gpc_op op, gpc_polygon *subj, gpc_polygon *clip,
                       gpc_tristrip *result) {}

void gpc_polygon_to_tristrip(gpc_polygon *s, gpc_tristrip *t) {
  gpc_polygon c;
  gpc_tristrip_clip(gpc_op::GPC_DIFF, s, &c, t);
}

} // namespace gpc

/*
===========================================================================
                           End of file: gpc.c
===========================================================================
*/
