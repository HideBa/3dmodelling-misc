
#include "bim_obj.cpp"
#include "cjson.cpp"
#include "io.cpp"
#include "types.h"
#include "voxelgrid.cpp"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

int main(int argc, const char *argv[]) {
//    vec<vec<string>> input_outputs = {
//            {"../../input/open_house_ifc4_2.obj",
//             "../../out/voxel.obj",
//             "../../out/out_open_house.city.json"},
//            {"../../input/wellness_center_sama.obj",
//             "../../out/voxel_wellness_sama.obj",
//             "../../out/wellness_center_sama.city.json"},
//            {"../../input/special_case1.obj",
//             "../../out/special_case1.obj",
//             "../../out/special_case1.json"},
//    };

//    for (const auto &input_output: input_outputs) {
//        std::cout << "Processing: " << input_output[0] << std::endl;
//        std::ifstream input;
//        input.open(input_output[0]);
//        auto [bim_objects, vertices] = read_obj(input);
//        input.close();
//
//        assign_semantics(bim_objects);
//
//        cout << "Reading obj finished :" << bim_objects.size() << "faces" << endl;
//        auto voxel_grid = create_voxel(vertices, 2, 0.2);
//        cout << "Finished creating voxel" << voxel_grid.voxels.size() << endl;
//        auto intersected_voxel_grid =
//                intersection_with_bim_obj(voxel_grid, bim_objects);
//
//        auto marked_voxel_grid = mark_exterior_interior(intersected_voxel_grid);
//
//        extract_surface(marked_voxel_grid);
//
//
//        bool res = write_voxel_obj(input_output[1], marked_voxel_grid);
//
//        json cj = export_voxel_to_cityjson(marked_voxel_grid);
//
//        write_json(cj, input_output[2]);
//    }


    const char *filename =
            (argc > 1) ? argv[1] : "../../input/open_house_ifc4.obj";
    std::cout << "Processing: " << filename << std::endl;
    std::ifstream input;

    input.open(filename);
    auto [bim_objects, vertices] = read_obj(input);
    input.close();

    assign_semantics(bim_objects);

    cout << "Reading obj finished :" << bim_objects.size() << "faces" << endl;
    auto voxel_grid = create_voxel(vertices, 2, 0.5);
    cout << "Finished creating voxel" << voxel_grid.voxels.size() << endl;
    auto intersected_voxel_grid =
            intersection_with_bim_obj(voxel_grid, bim_objects);

    auto marked_voxel_grid = mark_exterior_interior(intersected_voxel_grid);

    extract_surface(marked_voxel_grid);


    bool res = write_voxel_obj("out.obj", marked_voxel_grid);

    json cj = export_voxel_to_cityjson(marked_voxel_grid);

    write_json(cj, "out.city.json");

    return 0;
}
