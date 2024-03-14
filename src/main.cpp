#include <iostream>

#include "file_system.hpp"
#include "gpc.hpp"
#include "gpc_polygon.hpp"

int main(int argc, char **argv) {
  gpc::gpc_polygon subject_polygon;
  gpc::gpc_polygon clip_polygon;

  subject_polygon.add_contour(
      gpc::gpc_vertex_list({{0, 0}, {100, 0}, {100, 100}, {0, 100}}), false);

  clip_polygon.add_contour(
      gpc::gpc_vertex_list({{25, 25}, {50, 25}, {50, 50}, {25, 50}}), false);
  clip_polygon.add_contour(
      gpc::gpc_vertex_list({{50, 50}, {75, 50}, {75, 75}, {50, 75}}), false);

  gpc::gpc_polygon result_polygon;
  gpc::gpc_polygon_clip(gpc::gpc_op::GPC_DIFF, &subject_polygon, &clip_polygon,
                        &result_polygon);

  result_polygon.add_contour(
      gpc::gpc_vertex_list({{200, 200}, {300, 200}, {300, 300}, {200, 300}}),
      false);

  gpc::file_system::write_polygons("result.txt",
                                   {result_polygon, result_polygon});

  std::vector<gpc::gpc_polygon> read_polygons;
  gpc::file_system::read_polygons("result.txt", read_polygons);

  std::cout << "read_polygons.size(): " << read_polygons.size() << std::endl;

  for (int i = 0; i < read_polygons.size(); ++i) {
    std::cout << read_polygons[i].to_string() << std::endl;
  }

  return 0;
}
