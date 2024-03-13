void gpc_tristrip_clip(gpc_op op, gpc_polygon *subj, gpc_polygon *clip,
                       gpc_tristrip *result) {
  sb_tree *sbtree = nullptr;
  it_node *it = nullptr, *intersect;
  edge_node *edge, *prev_edge, *next_edge, *succ_edge, *e0, *e1;
  edge_node *aet = nullptr, *c_heap = nullptr, *s_heap = nullptr, *cf;
  lmt_node *lmt = nullptr, *local_min;
  polygon_node *tlist = nullptr, *tn, *tnn, *p, *q;
  vertex_node *lt, *ltn, *rt, *rtn;
  h_state horiz[2];
  vertex_type cft;
  int in[2], exists[2], parity[2] = {LEFT, LEFT};
  int s, v, contributing, scanbeam = 0, sbt_entries = 0;
  int vclass, bl, br, tl, tr;
  double *sbt = nullptr, xb, px, nx, yb, yt, dy, ix, iy;

  /* Test for trivial nullptr result cases */
  if (((subj->num_contours() == 0) && (clip->num_contours() == 0)) ||
      ((subj->num_contours() == 0) &&
       ((op == gpc_op::GPC_INT) || (op == gpc_op::GPC_DIFF))) ||
      ((clip->num_contours() == 0) && (op == gpc_op::GPC_INT))) {
    result->num_strips = 0;
    result->strip = nullptr;
    return;
  }

  /* Identify potentialy contributing contours */
  if (((op == gpc_op::GPC_INT) || (op == gpc_op::GPC_DIFF)) &&
      (subj->num_contours() > 0) && (clip->num_contours() > 0))
    minimax_test(subj, clip, op);

  /* Build LMT */
  if (subj->num_contours() > 0)
    s_heap = build_lmt(&lmt, &sbtree, &sbt_entries, subj, SUBJ, op);
  if (clip->num_contours() > 0)
    c_heap = build_lmt(&lmt, &sbtree, &sbt_entries, clip, CLIP, op);

  /* Return a nullptr result if no contours contribute */
  if (lmt == nullptr) {
    result->num_strips = 0;
    result->strip = nullptr;
    reset_lmt(&lmt);
    delete (s_heap);
    delete (c_heap);
    return;
  }

  /* Build scanbeam table from scanbeam tree */
  MALLOC(sbt, sbt_entries * sizeof(double), "sbt creation", double);
  build_sbt(&scanbeam, sbt, sbtree);
  scanbeam = 0;
  free_sbtree(&sbtree);

  /* Invert clip polygon for difference operation */
  if (op == gpc_op::GPC_DIFF)
    parity[CLIP] = RIGHT;

  local_min = lmt;

  /* Process each scanbeam */
  while (scanbeam < sbt_entries) {
    /* Set yb and yt to the bottom and top of the scanbeam */
    yb = sbt[scanbeam++];
    if (scanbeam < sbt_entries) {
      yt = sbt[scanbeam];
      dy = yt - yb;
    }

    /* === SCANBEAM BOUNDARY PROCESSING ================================ */

    /* If LMT node corresponding to yb exists */
    if (local_min) {
      if (local_min->y == yb) {
        /* Add edges starting at this local minimum to the AET */
        for (edge = local_min->first_bound; edge; edge = edge->next_bound)
          add_edge_to_aet(&aet, edge, nullptr);

        local_min = local_min->next;
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
            new_tristrip(&tlist, edge, xb, yb);
            cf = edge;
            break;
          case vertex_type::ERI:
            edge->outp[ABOVE] = cf->outp[ABOVE];
            if (xb != cf->xb)
              VERTEX(edge, ABOVE, RIGHT, xb, yb);
            cf = nullptr;
            break;
          case vertex_type::ELI:
            VERTEX(edge, BELOW, LEFT, xb, yb);
            edge->outp[ABOVE] = nullptr;
            cf = edge;
            break;
          case vertex_type::EMX:
            if (xb != cf->xb)
              VERTEX(edge, BELOW, RIGHT, xb, yb);
            edge->outp[ABOVE] = nullptr;
            cf = nullptr;
            break;
          case vertex_type::IMN:
            if (cft == vertex_type::LED) {
              if (cf->bot.y != yb)
                VERTEX(cf, BELOW, LEFT, cf->xb, yb);
              new_tristrip(&tlist, cf, cf->xb, yb);
            }
            edge->outp[ABOVE] = cf->outp[ABOVE];
            VERTEX(edge, ABOVE, RIGHT, xb, yb);
            break;
          case vertex_type::ILI:
            new_tristrip(&tlist, edge, xb, yb);
            cf = edge;
            cft = vertex_type::ILI;
            break;
          case vertex_type::IRI:
            if (cft == vertex_type::LED) {
              if (cf->bot.y != yb)
                VERTEX(cf, BELOW, LEFT, cf->xb, yb);
              new_tristrip(&tlist, cf, cf->xb, yb);
            }
            VERTEX(edge, BELOW, RIGHT, xb, yb);
            edge->outp[ABOVE] = nullptr;
            break;
          case vertex_type::IMX:
            VERTEX(edge, BELOW, LEFT, xb, yb);
            edge->outp[ABOVE] = nullptr;
            cft = vertex_type::IMX;
            break;
          case vertex_type::IMM:
            VERTEX(edge, BELOW, LEFT, xb, yb);
            edge->outp[ABOVE] = cf->outp[ABOVE];
            if (xb != cf->xb)
              VERTEX(cf, ABOVE, RIGHT, xb, yb);
            cf = edge;
            break;
          case vertex_type::EMM:
            VERTEX(edge, BELOW, RIGHT, xb, yb);
            edge->outp[ABOVE] = nullptr;
            new_tristrip(&tlist, edge, xb, yb);
            cf = edge;
            break;
          case vertex_type::LED:
            if (edge->bot.y == yb)
              VERTEX(edge, BELOW, LEFT, xb, yb);
            edge->outp[ABOVE] = edge->outp[BELOW];
            cf = edge;
            cft = vertex_type::LED;
            break;
          case vertex_type::RED:
            edge->outp[ABOVE] = cf->outp[ABOVE];
            if (cft == vertex_type::LED) {
              if (cf->bot.y == yb) {
                VERTEX(edge, BELOW, RIGHT, xb, yb);
              } else {
                if (edge->bot.y == yb) {
                  VERTEX(cf, BELOW, LEFT, cf->xb, yb);
                  VERTEX(edge, BELOW, RIGHT, xb, yb);
                }
              }
            } else {
              VERTEX(edge, BELOW, RIGHT, xb, yb);
              VERTEX(edge, ABOVE, RIGHT, xb, yb);
            }
            cf = nullptr;
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

    if (scanbeam < sbt_entries) {
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
            new_tristrip(&tlist, e1, ix, iy);
            e0->outp[ABOVE] = e1->outp[ABOVE];
            break;
          case vertex_type::ERI:
            if (p) {
              P_EDGE(prev_edge, e0, ABOVE, px, iy);
              VERTEX(prev_edge, ABOVE, LEFT, px, iy);
              VERTEX(e0, ABOVE, RIGHT, ix, iy);
              e1->outp[ABOVE] = e0->outp[ABOVE];
              e0->outp[ABOVE] = nullptr;
            }
            break;
          case vertex_type::ELI:
            if (q) {
              N_EDGE(next_edge, e1, ABOVE, nx, iy);
              VERTEX(e1, ABOVE, LEFT, ix, iy);
              VERTEX(next_edge, ABOVE, RIGHT, nx, iy);
              e0->outp[ABOVE] = e1->outp[ABOVE];
              e1->outp[ABOVE] = nullptr;
            }
            break;
          case vertex_type::EMX:
            if (p && q) {
              VERTEX(e0, ABOVE, LEFT, ix, iy);
              e0->outp[ABOVE] = nullptr;
              e1->outp[ABOVE] = nullptr;
            }
            break;
          case vertex_type::IMN:
            P_EDGE(prev_edge, e0, ABOVE, px, iy);
            VERTEX(prev_edge, ABOVE, LEFT, px, iy);
            N_EDGE(next_edge, e1, ABOVE, nx, iy);
            VERTEX(next_edge, ABOVE, RIGHT, nx, iy);
            new_tristrip(&tlist, prev_edge, px, iy);
            e1->outp[ABOVE] = prev_edge->outp[ABOVE];
            VERTEX(e1, ABOVE, RIGHT, ix, iy);
            new_tristrip(&tlist, e0, ix, iy);
            next_edge->outp[ABOVE] = e0->outp[ABOVE];
            VERTEX(next_edge, ABOVE, RIGHT, nx, iy);
            break;
          case vertex_type::ILI:
            if (p) {
              VERTEX(e0, ABOVE, LEFT, ix, iy);
              N_EDGE(next_edge, e1, ABOVE, nx, iy);
              VERTEX(next_edge, ABOVE, RIGHT, nx, iy);
              e1->outp[ABOVE] = e0->outp[ABOVE];
              e0->outp[ABOVE] = nullptr;
            }
            break;
          case vertex_type::IRI:
            if (q) {
              VERTEX(e1, ABOVE, RIGHT, ix, iy);
              P_EDGE(prev_edge, e0, ABOVE, px, iy);
              VERTEX(prev_edge, ABOVE, LEFT, px, iy);
              e0->outp[ABOVE] = e1->outp[ABOVE];
              e1->outp[ABOVE] = nullptr;
            }
            break;
          case vertex_type::IMX:
            if (p && q) {
              VERTEX(e0, ABOVE, RIGHT, ix, iy);
              VERTEX(e1, ABOVE, LEFT, ix, iy);
              e0->outp[ABOVE] = nullptr;
              e1->outp[ABOVE] = nullptr;
              P_EDGE(prev_edge, e0, ABOVE, px, iy);
              VERTEX(prev_edge, ABOVE, LEFT, px, iy);
              new_tristrip(&tlist, prev_edge, px, iy);
              N_EDGE(next_edge, e1, ABOVE, nx, iy);
              VERTEX(next_edge, ABOVE, RIGHT, nx, iy);
              next_edge->outp[ABOVE] = prev_edge->outp[ABOVE];
              VERTEX(next_edge, ABOVE, RIGHT, nx, iy);
            }
            break;
          case vertex_type::IMM:
            if (p && q) {
              VERTEX(e0, ABOVE, RIGHT, ix, iy);
              VERTEX(e1, ABOVE, LEFT, ix, iy);
              P_EDGE(prev_edge, e0, ABOVE, px, iy);
              VERTEX(prev_edge, ABOVE, LEFT, px, iy);
              new_tristrip(&tlist, prev_edge, px, iy);
              N_EDGE(next_edge, e1, ABOVE, nx, iy);
              VERTEX(next_edge, ABOVE, RIGHT, nx, iy);
              e1->outp[ABOVE] = prev_edge->outp[ABOVE];
              VERTEX(e1, ABOVE, RIGHT, ix, iy);
              new_tristrip(&tlist, e0, ix, iy);
              next_edge->outp[ABOVE] = e0->outp[ABOVE];
              VERTEX(next_edge, ABOVE, RIGHT, nx, iy);
            }
            break;
          case vertex_type::EMM:
            if (p && q) {
              VERTEX(e0, ABOVE, LEFT, ix, iy);
              new_tristrip(&tlist, e1, ix, iy);
              e0->outp[ABOVE] = e1->outp[ABOVE];
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

  /* Generate result tristrip from tlist */
  result->strip = nullptr;
  result->num_strips = count_tristrips(tlist);
  if (result->num_strips > 0) {
    MALLOC(result->strip, result->num_strips * sizeof(gpc_vertex_list),
           "tristrip list creation", gpc_vertex_list);

    s = 0;
    for (tn = tlist; tn; tn = tnn) {
      tnn = tn->next;

      if (tn->active > 2) {
        /* Valid tristrip: copy the vertices and free the heap */
        result->strip[s].vertex.resize(tn->active);

        v = 0;
        if (INVERT_TRISTRIPS) {
          lt = tn->v[RIGHT];
          rt = tn->v[LEFT];
        } else {
          lt = tn->v[LEFT];
          rt = tn->v[RIGHT];
        }
        while (lt || rt) {
          if (lt) {
            ltn = lt->next;
            result->strip[s].vertex[v].x = lt->x;
            result->strip[s].vertex[v].y = lt->y;
            v++;
            delete (lt);
            lt = ltn;
          }
          if (rt) {
            rtn = rt->next;
            result->strip[s].vertex[v].x = rt->x;
            result->strip[s].vertex[v].y = rt->y;
            v++;
            delete (rt);
            rt = rtn;
          }
        }
        s++;
      } else {
        /* Invalid tristrip: just free the heap */
        for (lt = tn->v[LEFT]; lt; lt = ltn) {
          ltn = lt->next;
          delete (lt);
        }
        for (rt = tn->v[RIGHT]; rt; rt = rtn) {
          rtn = rt->next;
          delete (rt);
        }
      }
      delete (tn);
    }
  }

  /* Tidy up */
  reset_it(&it);
  reset_lmt(&lmt);
  delete c_heap;
  delete s_heap;
  delete sbt;
}