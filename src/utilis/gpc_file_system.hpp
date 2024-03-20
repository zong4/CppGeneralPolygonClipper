#pragma once

#include <fstream>

#include "../geometry/gpc_polygon.hpp"

namespace gpc {

class gpc_file_system
{
public:
    // static file_system &instance() { return new file_system(); }

    static void read_polygons(const std::string &file_path,
                              std::vector<gpc_polygon> &polygons);
    static void write_polygons(const std::string &file_path,
                               const std::vector<gpc_polygon> &polygons);
};

} // namespace gpc