#ifdef DEBUGs

#include <gtest/gtest.h>
#include <iostream>

#include "gpc.hpp"
#include "utilis/gpc_file_system.hpp"

int main(int argc, char **argv)
{
    gpc::gpc_polygon subject_polygon;
    gpc::gpc_polygon clip_polygon;

    subject_polygon.add_contour(
        gpc::gpc_vertex_list({{0, 0}, {25, 25}, {0, 50}, {-25, 25}}));

    clip_polygon.add_contour(
        gpc::gpc_vertex_list({{0, 25}, {25, 50}, {0, 75}, {-25, 50}}));

    gpc::gpc_polygon result_polygon;
    gpc::gpc_polygon_clip(gpc::gpc_op::GPC_DIFF, subject_polygon, clip_polygon,
                          result_polygon);

    gpc::gpc_polygon expected_polygon;
    expected_polygon.add_contour(gpc::gpc_vertex_list({{0, 0},
                                                       {100, 0},
                                                       {100, 50},
                                                       {50, 50},
                                                       {50, 125},
                                                       {-25, 125},
                                                       {0, 100}}));

    std::cout << "result_polygon: " << result_polygon.to_string() << std::endl;
    std::cout << "expected_polygon: " << expected_polygon.to_string()
              << std::endl;

    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}

#endif