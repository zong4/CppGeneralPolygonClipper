#pragma once

#include "../utilis/gpc_enum.hpp"
#include "gpc_vertex.hpp"
#include "polygon_node.hpp"

namespace gpc {

// TODO:
class gpc_edge_node {
public:
  gpc_vertex vertex;             /* Piggy-backed contour vertex data  */
  gpc_vertex bot;                /* Edge lower (x, y) coordinate      */
  gpc_vertex top;                /* Edge upper (x, y) coordinate      */
  double xb = 0.0;               /* Scanbeam bottom x coordinate      */
  double xt = 0.0;               /* Scanbeam top x coordinate         */
  double dx = 0.0;               /* Change in x for a unit y increase */
  int type = 0;                  /* Clip / subject edge flag          */
  int bundle[2][2];              /* Bundle edge flags                 */
  int bside[2];                  /* Bundle left / right indicators    */
  bundle_state bstate[2];        /* Edge bundle state                 */
  polygon_node *outp[2];         /* Output polygon / tristrip pointer */
  gpc_edge_node *prev = nullptr; /* Previous edge in the AET          */
  gpc_edge_node *next = nullptr; /* Next edge in the AET              */
  gpc_edge_node *pred = nullptr; /* Edge connected at the lower end   */
  gpc_edge_node *succ = nullptr; /* Edge connected at the upper end   */

  gpc_edge_node() = default;
  ~gpc_edge_node() = default;
};

typedef std::list<gpc_edge_node> gpc_edge_list;

} // namespace gpc