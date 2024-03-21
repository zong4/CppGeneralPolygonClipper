#pragma once

#include <vector>
#include "geometry/gpc_vertex_list.hpp"
#include "geometry/it_node.hpp"
#include "geometry/polygon_node.hpp"
#include "geometry/st_node.hpp"
#include "utilis/gpc_constants.hpp"

namespace gpc {

static void reset_it(it_node **it)
{
    it_node *itn;

    while (*it)
    {
        itn = (*it)->next;
        delete *it;
        *it = itn;
    }
}

static void add_intersection(it_node **it, gpc_edge_node *edge0,
                             gpc_edge_node *edge1, double x, double y)
{
    it_node *existing_node;

    if (!*it)
    {
        // Append a new node to the tail of the list
        MALLOC(*it, sizeof(it_node), "IT insertion", it_node);
        (*it)->ie[0] = edge0;
        (*it)->ie[1] = edge1;
        (*it)->point.x = x;
        (*it)->point.y = y;
        (*it)->next = nullptr;
    }
    else
    {
        if ((*it)->point.y > y)
        {
            // Insert a new node mid-list
            existing_node = *it;
            MALLOC(*it, sizeof(it_node), "IT insertion", it_node);
            (*it)->ie[0] = edge0;
            (*it)->ie[1] = edge1;
            (*it)->point.x = x;
            (*it)->point.y = y;
            (*it)->next = existing_node;
        }
        else
            // Head further down the list
            add_intersection(&((*it)->next), edge0, edge1, x, y);
    }
}

static void add_st_edge(st_node **st, it_node **it, gpc_edge_node *edge,
                        double dy)
{
    st_node *existing_node;
    double den, r, x, y;

    if (!*st)
    {
        // Append edge onto the tail end of the ST
        MALLOC(*st, sizeof(st_node), "ST insertion", st_node);
        (*st)->edge = edge;
        (*st)->xb = edge->xb();
        (*st)->xt = edge->xt();
        (*st)->dx = edge->dx;
        (*st)->prev = nullptr;
    }
    else
    {
        den = ((*st)->xt - (*st)->xb) - (edge->xt() - edge->xb());

        // If new edge and ST edge don't cross
        if ((edge->xt() >= (*st)->xt) || (edge->dx == (*st)->dx) ||
            (fabs(den) <= DBL_EPSILON))
        {
            // No intersection - insert edge here (before the ST edge)
            existing_node = *st;
            MALLOC(*st, sizeof(st_node), "ST insertion", st_node);
            (*st)->edge = edge;
            (*st)->xb = edge->xb();
            (*st)->xt = edge->xt();
            (*st)->dx = edge->dx;
            (*st)->prev = existing_node;
        }
        else
        {
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
                                     std::list<gpc_edge_node> &aet, double dy)
{
    st_node *st, *stp;
    gpc_edge_node *edge;

    // Build intersection table for the current scanbeam
    reset_it(it);
    st = nullptr;

    // Process each AET edge
    for (auto &&edge : aet)
    {
        if ((edge.bstate[ABOVE] == bundle_state::BUNDLE_HEAD) ||
            edge.bundle[ABOVE][CLIP] || edge.bundle[ABOVE][SUBJ])
            add_st_edge(&st, it, &edge, dy);
    }

    // Free the sorted edge table
    while (st)
    {
        stp = st->prev;
        delete (st);
        st = stp;
    }
}

// TODO:
static void swap_intersecting_edge_bundles(std::list<gpc_edge_node> &aet,
                                           it_node *intersect)
{
    gpc_edge_node *e0 = intersect->ie[0];
    gpc_edge_node *e1 = intersect->ie[1];

    // Find the iterators for e0 and e1 in the list
    auto e0_it = std::find(aet.begin(), aet.end(), *e0);
    auto e1_it = std::find(aet.begin(), aet.end(), *e1);

    // Check if e0 and e1 are found in the list
    if (e0_it != aet.end() && e1_it != aet.end())
    {
        // Find the iterator before the e0 bundle
        auto e0p_it = e0_it;
        if (e0_it != aet.begin() &&
            e0->bstate[ABOVE] == bundle_state::BUNDLE_HEAD)
        {
            do
            {
                e0p_it = std::prev(e0p_it);
            } while (e0p_it != aet.begin() &&
                     e0p_it->bstate[ABOVE] == bundle_state::BUNDLE_TAIL);
        }

        // Find the iterator before the e1 bundle
        auto e1p_it = e1_it;
        if (e1_it != aet.begin() &&
            e1->bstate[ABOVE] == bundle_state::BUNDLE_HEAD)
        {
            do
            {
                e1p_it = std::prev(e1p_it);
            } while (e1p_it != aet.begin() &&
                     e1p_it->bstate[ABOVE] == bundle_state::BUNDLE_TAIL);
        }

        // Swap the e0p and e1p links
        if (e0p_it != e1_it)
        {
            std::iter_swap(e0p_it, e1p_it);
        }

        // Re-link after e0
        if (std::next(e0_it) != e1_it)
        {
            aet.insert(std::next(e0_it), *e1_it);
            aet.erase(e1_it);
        }

        // Re-link after e1
        if (std::next(e1_it) != e0_it)
        {
            aet.insert(std::next(e1_it), *e0_it);
            aet.erase(e0_it);
        }
    }
}

static void add_local_min(std::vector<polygon_node *> &p, gpc_edge_node *edge,
                          double x, double y)
{
    polygon_node *polygon = new polygon_node(gpc_vertex_list({{x, y}}, false));
    polygon->proxy = polygon;

    p.insert(p.begin(), polygon);

    // Assign polygon p to the edge
    edge->outp[ABOVE] = polygon;
}

} // namespace gpc