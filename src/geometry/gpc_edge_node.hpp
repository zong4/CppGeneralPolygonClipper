#pragma once

#include <list>

#include "../utilis/gpc_enum.hpp"
#include "gpc_vertex.hpp"
#include "polygon_node.hpp"

namespace gpc {

// TODO:
class gpc_edge_node
{
public:
    gpc_vertex bot; // Edge lower (x, y) coordinate
    gpc_vertex top; // Edge upper (x, y) coordinate

    double dx = 0.0; // Change in x for a unit y increase

    bool type = false; // Clip / subject edge flag

    int bundle[2][2] = {{false, false}, {false, false}}; // Bundle edge flags

    int bside[2] = {false, false}; // Bundle left / right indicators

    bundle_state bstate[2] = {bundle_state::UNBUNDLED,
                              bundle_state::UNBUNDLED}; // Edge bundle state

    polygon_node *outp[2]; // Output polygon / tristrip pointer

    gpc_edge_node *pred = nullptr; // Edge connected at the lower end
    gpc_edge_node *succ = nullptr; // Edge connected at the upper end

public:
    gpc_edge_node() = default;
    ~gpc_edge_node() = default;

    inline double xb() const { return xbot; }
    inline double xt() const { return xtop; }

    inline void xb(double value) { xbot = value; }
    inline void xt(double value) { xtop = value; }

    inline bool operator==(const gpc_edge_node &rhs) const
    {
        return (bot == rhs.bot) && (top == rhs.top) && (type == rhs.type);
    }

private:
    double xbot = 0.0; // Scanbeam bottom x coordinate
    double xtop = 0.0; // Scanbeam top x coordinate
};

typedef std::list<gpc_edge_node> gpc_edge_list;

} // namespace gpc