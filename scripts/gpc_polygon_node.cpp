// #include "gpc_polygon_node.hpp"

// gpc::gpc_polygon_node::gpc_polygon_node(const gpc::gpc_vertex &vertex,
//                                         int hole_flag, int active_flag)
//     : active(active_flag), hole(hole_flag)
// {
//     vertex_list.push_back(vertex);
// }

// void gpc::gpc_polygon_node::add_left(const gpc_vertex &vertex)
// {
//     // Add vertex nv to the left end of the polygon's vertex list
//     vertex_list.push_front(vertex);
// }

// void gpc::gpc_polygon_node::add_right(const gpc_vertex &vertex)
// {
//     // Add vertex nv to the right end of the polygon's vertex list
//     vertex_list.push_back(vertex);
// }

// void gpc::merge_left(gpc_polygon_node &p, gpc_polygon_node &q,
//                      std::list<gpc_polygon_node> &list)
// {
//     // Label contour as a hole
//     q.hole = true;

//     if (p != q && p.active && q.active)
//     {
//         // Assign p's vertex list to the left end of q's list
//         p.vertex_list.splice(p.vertex_list.end(), q.vertex_list);
//         q.vertex_list = p.vertex_list;

//         // Redirect any p->proxy references to q->proxy
//         q.active = false;
//     }
// }

// void gpc::merge_right(gpc_polygon_node &p, gpc_polygon_node &q,
//                       std::list<gpc_polygon_node> &list)
// {
//     // Label contour as external
//     q.hole = false;

//     if (p != q && p.active && q.active)
//     {
//         // Assign p's vertex list to the right end of q's list
//         q.vertex_list.splice(q.vertex_list.end(), p.vertex_list);

//         // Redirect any p->proxy references to q->proxy
//         p.active = false;
//     }
// }