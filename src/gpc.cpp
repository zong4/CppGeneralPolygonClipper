#include "gpc.hpp"

namespace gpc {

static void reset_it(it_node **it) {
  it_node *itn;

  while (*it) {
    itn = (*it)->next;
    delete *it;
    *it = itn;
  }
}

static void add_intersection(it_node **it, gpc_edge_node *edge0,
                             gpc_edge_node *edge1, double x, double y) {
  it_node *existing_node;

  if (!*it) {
    // Append a new node to the tail of the list
    MALLOC(*it, sizeof(it_node), "IT insertion", it_node);
    (*it)->ie[0] = edge0;
    (*it)->ie[1] = edge1;
    (*it)->point.x = x;
    (*it)->point.y = y;
    (*it)->next = nullptr;
  } else {
    if ((*it)->point.y > y) {
      // Insert a new node mid-list
      existing_node = *it;
      MALLOC(*it, sizeof(it_node), "IT insertion", it_node);
      (*it)->ie[0] = edge0;
      (*it)->ie[1] = edge1;
      (*it)->point.x = x;
      (*it)->point.y = y;
      (*it)->next = existing_node;
    } else
      // Head further down the list
      add_intersection(&((*it)->next), edge0, edge1, x, y);
  }
}

static void add_st_edge(st_node **st, it_node **it, gpc_edge_node *edge,
                        double dy) {
  st_node *existing_node;
  double den, r, x, y;

  if (!*st) {
    // Append edge onto the tail end of the ST
    MALLOC(*st, sizeof(st_node), "ST insertion", st_node);
    (*st)->edge = edge;
    (*st)->xb = edge->xb();
    (*st)->xt = edge->xt();
    (*st)->dx = edge->dx;
    (*st)->prev = nullptr;
  } else {
    den = ((*st)->xt - (*st)->xb) - (edge->xt() - edge->xb());

    // If new edge and ST edge don't cross
    if ((edge->xt() >= (*st)->xt) || (edge->dx == (*st)->dx) ||
        (fabs(den) <= DBL_EPSILON)) {
      // No intersection - insert edge here (before the ST edge)
      existing_node = *st;
      MALLOC(*st, sizeof(st_node), "ST insertion", st_node);
      (*st)->edge = edge;
      (*st)->xb = edge->xb();
      (*st)->xt = edge->xt();
      (*st)->dx = edge->dx;
      (*st)->prev = existing_node;
    } else {
      // Compute intersection between new edge and ST edge
      r = (edge->xb() - (*st)->xb) / den;
      x = (*st)->xb + r * ((*st)->xt - (*st)->xb);
      y = r * dy;

      // Insert the edge pointers and the intersection point in the IT
      add_intersection(it, (*st)->edge, edge, x, y);

      // Head further into the ST
      add_st_edge(&((*st)->prev), it, edge, dy);
    }
  }
}

static void build_intersection_table(it_node **it,
                                     std::list<gpc_edge_node> &aet, double dy) {
  st_node *st, *stp;
  gpc_edge_node *edge;

  // Build intersection table for the current scanbeam
  reset_it(it);
  st = nullptr;

  // Process each AET edge
  for (auto &&edge : aet) {
    if ((edge.bstate[ABOVE] == bundle_state::BUNDLE_HEAD) ||
        edge.bundle[ABOVE][CLIP] || edge.bundle[ABOVE][SUBJ])
      add_st_edge(&st, it, &edge, dy);
  }

  // Free the sorted edge table
  while (st) {
    stp = st->prev;
    delete (st);
    st = stp;
  }
}

// TODO:
static void swap_intersecting_edge_bundles(std::list<gpc_edge_node> &aet,
                                           it_node *intersect) {
  gpc_edge_node *e0 = intersect->ie[0];
  gpc_edge_node *e1 = intersect->ie[1];

  // Find the iterators for e0 and e1 in the list
  auto e0_it = std::find(aet.begin(), aet.end(), *e0);
  auto e1_it = std::find(aet.begin(), aet.end(), *e1);

  // Check if e0 and e1 are found in the list
  if (e0_it != aet.end() && e1_it != aet.end()) {
    // Find the iterator before the e0 bundle
    auto e0p_it = e0_it;
    if (e0_it != aet.begin() &&
        e0->bstate[ABOVE] == bundle_state::BUNDLE_HEAD) {
      do {
        e0p_it = std::prev(e0p_it);
      } while (e0p_it != aet.begin() &&
               e0p_it->bstate[ABOVE] == bundle_state::BUNDLE_TAIL);
    }

    // Find the iterator before the e1 bundle
    auto e1p_it = e1_it;
    if (e1_it != aet.begin() &&
        e1->bstate[ABOVE] == bundle_state::BUNDLE_HEAD) {
      do {
        e1p_it = std::prev(e1p_it);
      } while (e1p_it != aet.begin() &&
               e1p_it->bstate[ABOVE] == bundle_state::BUNDLE_TAIL);
    }

    // Swap the e0p and e1p links
    if (e0p_it != e1_it) {
      std::iter_swap(e0p_it, e1p_it);
    }

    // Re-link after e0
    if (std::next(e0_it) != e1_it) {
      aet.insert(std::next(e0_it), *e1_it);
      aet.erase(e1_it);
    }

    // Re-link after e1
    if (std::next(e1_it) != e0_it) {
      aet.insert(std::next(e1_it), *e0_it);
      aet.erase(e0_it);
    }
  }
}

static int count_contours(polygon_node *polygon) {
  int nc, nv;
  vertex_node *v, *nextv;

  for (nc = 0; polygon; polygon = polygon->next)
    if (polygon->active) {
      // Count the vertices in the current contour
      nv = 0;
      for (v = polygon->proxy->v[LEFT]; v; v = v->next)
        nv++;

      // Record valid vertex counts in the active field
      if (nv > 2) {
        polygon->active = nv;
        nc++;
      } else {
        // Invalid contour: just free the heap
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

  // Create a new vertex node and set its fields
  MALLOC(nv, sizeof(vertex_node), "vertex node creation", vertex_node);
  nv->x = x;
  nv->y = y;

  // Add vertex nv to the left end of the polygon's vertex list
  nv->next = p->proxy->v[LEFT];

  // Update proxy->[LEFT] to point to nv
  p->proxy->v[LEFT] = nv;
}

static void merge_left(polygon_node *p, polygon_node *q, polygon_node *list) {
  polygon_node *target;

  // Label contour as a hole
  q->proxy->hole = TRUE;

  if (p->proxy != q->proxy) {
    // Assign p's vertex list to the left end of q's list
    p->proxy->v[RIGHT]->next = q->proxy->v[LEFT];
    q->proxy->v[LEFT] = p->proxy->v[LEFT];

    // Redirect any p->proxy references to q->proxy

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

  // Create a new vertex node and set its fields
  MALLOC(nv, sizeof(vertex_node), "vertex node creation", vertex_node);
  nv->x = x;
  nv->y = y;
  nv->next = nullptr;

  // Add vertex nv to the right end of the polygon's vertex list
  p->proxy->v[RIGHT]->next = nv;

  // Update proxy->v[RIGHT] to point to nv
  p->proxy->v[RIGHT] = nv;
}

static void merge_right(polygon_node *p, polygon_node *q, polygon_node *list) {
  polygon_node *target;

  // Label contour as external
  q->proxy->hole = FALSE;

  if (p->proxy != q->proxy) {
    // Assign p's vertex list to the right end of q's list
    q->proxy->v[RIGHT]->next = p->proxy->v[LEFT];
    q->proxy->v[RIGHT] = p->proxy->v[RIGHT];

    // Redirect any p->proxy references to q->proxy
    for (target = p->proxy; list; list = list->next) {
      if (list->proxy == target) {
        list->active = FALSE;
        list->proxy = q->proxy;
      }
    }
  }
}

static void add_local_min(polygon_node **p, gpc_edge_node *edge, double x,
                          double y) {
  polygon_node *existing_min;
  vertex_node *nv;

  existing_min = *p;

  MALLOC(*p, sizeof(polygon_node), "polygon node creation", polygon_node);

  // Create a new vertex node and set its fields
  MALLOC(nv, sizeof(vertex_node), "vertex node creation", vertex_node);
  nv->x = x;
  nv->y = y;
  nv->next = nullptr;

  // Initialise proxy to point to p itself
  (*p)->proxy = (*p);
  (*p)->active = TRUE;
  (*p)->next = existing_min;

  // Make v[LEFT] and v[RIGHT] point to new vertex nv
  (*p)->v[LEFT] = nv;
  (*p)->v[RIGHT] = nv;

  // Assign polygon p to the edge
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
    // Head further down the list
    add_vertex(&((*t)->next), x, y);
}

static void new_tristrip(polygon_node **tn, gpc_edge_node *edge, double x,
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
    // Head further down the list
    new_tristrip(&((*tn)->next), edge, x, y);
}

void gpc_polygon_clip(gpc_op op, gpc_polygon &subj, gpc_polygon &clip,
                      gpc_polygon &result) {
  // clear result
  result.contour.clear();

  if (subj.num_contours() == 0 && clip.num_contours() == 0) {
    return;
  }

  // TODO: 无自相交
  if (clip.num_contours() == 0) {
    if (op == gpc_op::GPC_INT) {
      return;
    } else {
      result = subj;
      return;
    }
  }

  if (subj.num_contours() == 0) {
    if ((op == gpc_op::GPC_INT) || (op == gpc_op::GPC_DIFF)) {
      return;
    } else {
      result = clip;
      return;
    }
  }

  // subj 和 clip 都不为空
  // 确定可能有贡献的轮廓
  if ((op == gpc_op::GPC_INT) || (op == gpc_op::GPC_DIFF)) {
    minimax_test(subj, clip, op);
  }

  // 构建局部最小表
  gpc_lmt lmt;
  lmt.build_lmt(subj, SUBJ, op);
  lmt.build_lmt(clip, CLIP, op);

  std::sort(lmt.sbtree.begin(), lmt.sbtree.end());
  const std::vector<double> &sbt = lmt.sbtree;

  // 如果没有轮廓有贡献，返回空结果
  if (lmt.lmt_list.empty()) {
    return;
  }

  // 对于差集操作，反转剪切多边形
  int parity[2] = {LEFT, LEFT};
  if (op == gpc_op::GPC_DIFF)
    parity[CLIP] = RIGHT;

  it_node *it = nullptr, *intersect;
  polygon_node *out_poly = nullptr, *p, *q, *poly, *npoly;
  int in[2];
  polygon_node *cf = nullptr; // TODO:

  // 处理每个扫描线
  int scanbeam = 0;
  gpc_aet aet;
  while (scanbeam < sbt.size()) {
    // 设置扫描线的底部和顶部
    double yb = sbt[scanbeam++];

    while (scanbeam < sbt.size()) {
      if (sbt[scanbeam] == yb)
        ++scanbeam;
      else
        break;
    }

    double yt = sbt[scanbeam];
    double dy = yt - yb;

    // 扫描线边界处理
    // 将从这个局部最小值开始的边添加到 AET 中
    if (!lmt.lmt_list.empty()) {
      if (lmt.lmt_list.front().first == yb) {
        // Add edges starting at this local minimum to the AET
        for (auto &&edge : lmt.lmt_list.front().second) {
          aet.insert(edge);
        }

        lmt.lmt_list.pop_front(); // 移动到下一个局部最小值
      }
    }

    // 在 AET 中创建捆绑
    // 为第一条边设置捆绑字段
    aet.aet_list.front().bundle[ABOVE][aet.aet_list.front().type] =
        (aet.aet_list.front().top.y !=
         yb); // 如果边的顶部不在当前扫描线上，则设置为 TRUE
    aet.aet_list.front().bundle[ABOVE][!aet.aet_list.front().type] = FALSE;
    aet.aet_list.front().bstate[ABOVE] = bundle_state::UNBUNDLED;

    for (auto it = std::next(aet.aet_list.begin()); it != aet.aet_list.end();
         ++it) {
      // Set up bundle fields of next edge
      it->bundle[ABOVE][it->type] = (it->top.y != yb);
      it->bundle[ABOVE][!it->type] = FALSE;
      it->bstate[ABOVE] = bundle_state::UNBUNDLED;

      // 如果边在扫描线边界以上且与前一条边重合，则捆绑边
      if (it->bundle[ABOVE][it->type]) {
        if (equal(std::prev(it)->xb(), it->xb()) &&
            equal(std::prev(it)->dx, it->dx) && (std::prev(it)->top.y != yb)) {
          it->bundle[ABOVE][it->type] ^= std::prev(it)->bundle[ABOVE][it->type];
          it->bundle[ABOVE][!it->type] =
              std::prev(it)->bundle[ABOVE][!it->type];
          it->bstate[ABOVE] = bundle_state::BUNDLE_HEAD;

          std::prev(it)->bundle[ABOVE][CLIP] = FALSE;
          std::prev(it)->bundle[ABOVE][SUBJ] = FALSE;
          std::prev(it)->bstate[ABOVE] = bundle_state::BUNDLE_TAIL;
        }
      }
    }

    // 设置虚拟的前一个 x 值
    double px = -DBL_MAX;

    int exists[2];

    // 设置多边形的水平状态为非水平
    h_state horiz[2] = {h_state::NH, h_state::NH};

    // 在此扫描线边界处理每条边
    for (auto &&edge : aet.aet_list) {
      exists[CLIP] = edge.bundle[ABOVE][CLIP] + (edge.bundle[BELOW][CLIP] << 1);
      exists[SUBJ] = edge.bundle[ABOVE][SUBJ] + (edge.bundle[BELOW][SUBJ] << 1);

      if (exists[CLIP] || exists[SUBJ]) {
        // 设置边的边界侧
        edge.bside[CLIP] = parity[CLIP];
        edge.bside[SUBJ] = parity[SUBJ];

        // Determine contributing status and quadrant occupancies
        int contributing;
        int bl, br, tl, tr;
        switch (op) {
        case gpc_op::GPC_DIFF:
        case gpc_op::GPC_INT:
          contributing =
              (exists[CLIP] && (parity[SUBJ] || horiz[SUBJ])) ||
              (exists[SUBJ] && (parity[CLIP] || horiz[CLIP])) ||
              (exists[CLIP] && exists[SUBJ] && (parity[CLIP] == parity[SUBJ]));

          br = (parity[CLIP]) && (parity[SUBJ]);

          bl = (parity[CLIP] ^ edge.bundle[ABOVE][CLIP]) &&
               (parity[SUBJ] ^ edge.bundle[ABOVE][SUBJ]);

          tr = (parity[CLIP] ^ (horiz[CLIP] != h_state::NH)) &&
               (parity[SUBJ] ^ (horiz[SUBJ] != h_state::NH));

          tl = (parity[CLIP] ^ (horiz[CLIP] != h_state::NH) ^
                edge.bundle[BELOW][CLIP]) &&
               (parity[SUBJ] ^ (horiz[SUBJ] != h_state::NH) ^
                edge.bundle[BELOW][SUBJ]);
          break;
        case gpc_op::GPC_XOR:
          contributing = exists[CLIP] || exists[SUBJ];

          br = (parity[CLIP]) ^ (parity[SUBJ]);

          bl = (parity[CLIP] ^ edge.bundle[ABOVE][CLIP]) ^
               (parity[SUBJ] ^ edge.bundle[ABOVE][SUBJ]);

          tr = (parity[CLIP] ^ (horiz[CLIP] != h_state::NH)) ^
               (parity[SUBJ] ^ (horiz[SUBJ] != h_state::NH));

          tl = (parity[CLIP] ^ (horiz[CLIP] != h_state::NH) ^
                edge.bundle[BELOW][CLIP]) ^
               (parity[SUBJ] ^ (horiz[SUBJ] != h_state::NH) ^
                edge.bundle[BELOW][SUBJ]);
          break;
        case gpc_op::GPC_UNION:
          contributing =
              (exists[CLIP] && (!parity[SUBJ] || horiz[SUBJ])) ||
              (exists[SUBJ] && (!parity[CLIP] || horiz[CLIP])) ||
              (exists[CLIP] && exists[SUBJ] && (parity[CLIP] == parity[SUBJ]));

          br = (parity[CLIP]) || (parity[SUBJ]);

          bl = (parity[CLIP] ^ edge.bundle[ABOVE][CLIP]) ||
               (parity[SUBJ] ^ edge.bundle[ABOVE][SUBJ]);

          tr = (parity[CLIP] ^ (horiz[CLIP] != h_state::NH)) ||
               (parity[SUBJ] ^ (horiz[SUBJ] != h_state::NH));

          tl = (parity[CLIP] ^ (horiz[CLIP] != h_state::NH) ^
                edge.bundle[BELOW][CLIP]) ||
               (parity[SUBJ] ^ (horiz[SUBJ] != h_state::NH) ^
                edge.bundle[BELOW][SUBJ]);
          break;
        }

        // 更新奇偶性
        parity[CLIP] ^= edge.bundle[ABOVE][CLIP];
        parity[SUBJ] ^= edge.bundle[ABOVE][SUBJ];

        // 更新水平状态
        if (exists[CLIP])
          horiz[CLIP] = next_h_state[horiz[CLIP]]
                                    [((exists[CLIP] - 1) << 1) + parity[CLIP]];
        if (exists[SUBJ])
          horiz[SUBJ] = next_h_state[horiz[SUBJ]]
                                    [((exists[SUBJ] - 1) << 1) + parity[SUBJ]];

        vertex_type vclass =
            static_cast<vertex_type>(tr + (tl << 1) + (br << 2) + (bl << 3));

        if (contributing) {
          double xb = edge.xb();

          switch (vclass) {
          case vertex_type::EMN: // 外部最小值
          case vertex_type::IMN: // 内部最小值
            add_local_min(&out_poly, &edge, xb, yb);
            px = xb;
            cf = edge.outp[ABOVE];
            break;
          case vertex_type::ERI: // 外部右中间值
            if (xb != px) {
              add_right(cf, xb, yb);
              px = xb;
            }

            edge.outp[ABOVE] = cf;
            cf = nullptr;
            break;
          case vertex_type::ELI: // 外部左中间值
            add_left(edge.outp[BELOW], xb, yb);
            px = xb;
            cf = edge.outp[BELOW];
            break;
          case vertex_type::EMX: // 外部最大值
            if (xb != px) {
              add_left(cf, xb, yb);
              px = xb;
            }

            merge_right(cf, edge.outp[BELOW], out_poly);
            cf = nullptr;
            break;
          case vertex_type::ILI: // 内部左中间值
            if (xb != px) {
              add_left(cf, xb, yb);
              px = xb;
            }

            edge.outp[ABOVE] = cf;
            cf = nullptr;
            break;
          case vertex_type::IRI: // 内部右中间值
            add_right(edge.outp[BELOW], xb, yb);
            px = xb;
            cf = edge.outp[BELOW];
            edge.outp[BELOW] = nullptr;
            break;
          case vertex_type::IMX: // 内部最大值
            if (xb != px) {
              add_right(cf, xb, yb);
              px = xb;
            }

            merge_left(cf, edge.outp[BELOW], out_poly);
            cf = nullptr;
            edge.outp[BELOW] = nullptr;
            break;
          case vertex_type::IMM: // 内部最大值和最小值
            if (xb != px) {
              add_right(cf, xb, yb);
              px = xb;
            }

            merge_left(cf, edge.outp[BELOW], out_poly);
            edge.outp[BELOW] = nullptr;
            add_local_min(&out_poly, &edge, xb, yb);
            cf = edge.outp[ABOVE];
            break;
          case vertex_type::EMM: // 外部最大值和最小值
            if (xb != px) {
              add_left(cf, xb, yb);
              px = xb;
            }

            merge_right(cf, edge.outp[BELOW], out_poly);
            edge.outp[BELOW] = nullptr;
            add_local_min(&out_poly, &edge, xb, yb);
            cf = edge.outp[ABOVE];
            break;
          case vertex_type::LED: // 左边界
            if (edge.bot.y == yb) {
              add_left(edge.outp[BELOW], xb, yb);
            }

            edge.outp[ABOVE] = edge.outp[BELOW];
            px = xb;
            break;
          case vertex_type::RED: // 右边界
            if (edge.bot.y == yb) {
              add_right(edge.outp[BELOW], xb, yb);
            }

            edge.outp[ABOVE] = edge.outp[BELOW];
            px = xb;
            break;
          default:
            break;
          } // End of switch
        }   // End of contributing conditional
      }     // End of edge exists conditional
    }       // End of AET loop

    // Delete terminating edges from the AET, otherwise compute xt()
    for (auto it = aet.aet_list.begin(); it != aet.aet_list.end();) {
      if (it->top.y == yb) {
        // Copy bundle head state to the adjacent tail edge if required
        if ((it != aet.aet_list.begin() &&
             it->bstate[BELOW] == bundle_state::BUNDLE_HEAD) &&
            std::prev(it)->bstate[BELOW] == bundle_state::BUNDLE_TAIL) {
          std::prev(it)->outp[BELOW] = it->outp[BELOW];
          std::prev(it)->bstate[BELOW] = bundle_state::UNBUNDLED;

          if (std::prev(it) != aet.aet_list.begin() &&
              std::prev(std::prev(it))->bstate[BELOW] ==
                  bundle_state::BUNDLE_TAIL) {
            std::prev(it)->bstate[BELOW] = bundle_state::BUNDLE_HEAD;
          }
        }

        it = aet.aet_list.erase(it);
      } else {
        // 更新 xtop
        if (it->top.y == yt) {
          it->xt(it->top.x);
        } else {
          it->xt(it->bot.x + it->dx * (yt - it->bot.y));
        }

        ++it;
      }
    }

    if (scanbeam < sbt.size()) {
      // SCANBEAM INTERIOR PROCESSING
      build_intersection_table(&it, aet.aet_list, dy);

      // Process each node in the intersection table
      for (intersect = it; intersect; intersect = intersect->next) {
        gpc_edge_node *e0 = intersect->ie[0];
        gpc_edge_node *e1 = intersect->ie[1];

        // Only generate output for contributing intersections
        if ((e0->bundle[ABOVE][CLIP] || e0->bundle[ABOVE][SUBJ]) &&
            (e1->bundle[ABOVE][CLIP] || e1->bundle[ABOVE][SUBJ])) {
          p = e0->outp[ABOVE];
          q = e1->outp[ABOVE];
          double ix = intersect->point.x;
          double iy = intersect->point.y + yb;

          in[CLIP] = (e0->bundle[ABOVE][CLIP] && !e0->bside[CLIP]) ||
                     (e1->bundle[ABOVE][CLIP] && e1->bside[CLIP]) ||
                     (!e0->bundle[ABOVE][CLIP] && !e1->bundle[ABOVE][CLIP] &&
                      e0->bside[CLIP] && e1->bside[CLIP]);
          in[SUBJ] = (e0->bundle[ABOVE][SUBJ] && !e0->bside[SUBJ]) ||
                     (e1->bundle[ABOVE][SUBJ] && e1->bside[SUBJ]) ||
                     (!e0->bundle[ABOVE][SUBJ] && !e1->bundle[ABOVE][SUBJ] &&
                      e0->bside[SUBJ] && e1->bside[SUBJ]);

          // Determine quadrant occupancies
          int bl, br, tl, tr;
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

          vertex_type vclass =
              static_cast<vertex_type>(tr + (tl << 1) + (br << 2) + (bl << 3));

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
          } // End of switch
        }   // End of contributing intersection conditional

        // Swap bundle sides in response to edge crossing
        if (e0->bundle[ABOVE][CLIP])
          e1->bside[CLIP] = !e1->bside[CLIP];
        if (e1->bundle[ABOVE][CLIP])
          e0->bside[CLIP] = !e0->bside[CLIP];
        if (e0->bundle[ABOVE][SUBJ])
          e1->bside[SUBJ] = !e1->bside[SUBJ];
        if (e1->bundle[ABOVE][SUBJ])
          e0->bside[SUBJ] = !e0->bside[SUBJ];

        // Swap the edge bundles in the aet.aet_list.
        swap_intersecting_edge_bundles(aet.aet_list, intersect);

      } // End of IT loop

      // Prepare for next scanbeam
      for (auto it = aet.aet_list.begin(); it != aet.aet_list.end();) {
        gpc_edge_node *succ_edge = it->succ;

        if (it->top.y == yt && succ_edge) {
          // Replace AET edge by its successor
          succ_edge->outp[BELOW] = it->outp[ABOVE];
          succ_edge->bstate[BELOW] = it->bstate[ABOVE];
          succ_edge->bundle[BELOW][CLIP] = it->bundle[ABOVE][CLIP];
          succ_edge->bundle[BELOW][SUBJ] = it->bundle[ABOVE][SUBJ];

          aet.aet_list.insert(it, *succ_edge);
          it = aet.aet_list.erase(it);
        } else {
          // Update this edge
          it->xb(it->xt());

          it->outp[BELOW] = it->outp[ABOVE];
          it->outp[ABOVE] = nullptr;

          it->bstate[BELOW] = it->bstate[ABOVE];

          it->bundle[BELOW][CLIP] = it->bundle[ABOVE][CLIP];
          it->bundle[BELOW][SUBJ] = it->bundle[ABOVE][SUBJ];

          ++it;
        }
      }
    }
  } // END OF SCANBEAM PROCESSING

  // Generate result polygon from out_poly
  if (count_contours(out_poly) > 0) {
    result.hole.resize(count_contours(out_poly));
    result.contour.resize(count_contours(out_poly));

    int c = 0;
    for (poly = out_poly; poly; poly = npoly) {
      npoly = poly->next;
      if (poly->active) {
        result.hole[c] = poly->proxy->hole;

        result.contour[c].vertex.resize(poly->active);

        int v = result.contour[c].vertex.size() - 1;

        vertex_node *nv = nullptr;
        for (vertex_node *vtx = poly->proxy->v[LEFT]; vtx; vtx = nv) {
          vertex_node *nv = vtx->next;

          result.contour[c].vertex[v].x = vtx->x;
          result.contour[c].vertex[v].y = vtx->y;
          delete (vtx);

          --v;
        }
        ++c;
      }
      delete poly;
    }
  } else {
    for (poly = out_poly; poly; poly = npoly) {
      npoly = poly->next;
      delete (poly);
    }
  }

  // Tidy up
  reset_it(&it);
}

void gpc_tristrip_clip(gpc_op op, gpc_polygon *subj, gpc_polygon *clip,
                       gpc_tristrip *result) {}

void gpc_polygon_to_tristrip(gpc_polygon *s, gpc_tristrip *t) {
  gpc_polygon c;
  gpc_tristrip_clip(gpc_op::GPC_DIFF, s, &c, t);
}

} // namespace gpc
