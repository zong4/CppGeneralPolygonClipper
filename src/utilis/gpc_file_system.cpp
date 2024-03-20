#include "gpc_file_system.hpp"

void gpc::gpc_file_system::read_polygons(
    const std::string &file_path, std::vector<gpc::gpc_polygon> &polygons)
{
    // open file
    std::ifstream file(file_path);

    while (file)
    {
        gpc::gpc_polygon polygon;
        file >> polygon;

        if (polygon.num_contours() == 0)
        {
            break;
        }

        polygons.push_back(polygon);
    }

    file.close();
}

void gpc::gpc_file_system::write_polygons(
    const std::string &file_path, const std::vector<gpc::gpc_polygon> &polygons)
{
    // open or create file
    std::ofstream file(file_path);

    for (int i = 0; i < polygons.size(); ++i)
    {
        file << polygons[i] << "\n";
    }

    file.close();
}
