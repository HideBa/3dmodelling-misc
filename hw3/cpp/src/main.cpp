
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
  vec<pair<string, string>> input_outputs = {

      {"../../input/open_house_ifc4.obj", "../../out/out_open_house.city.json"},
  };

  for (const auto &input_output : input_outputs) {
    std::cout << "Processing: " << input_output.first << std::endl;
    std::ifstream input;
    input.open(input_output.first);
    auto [bim_objects, vertices] = read_obj(input);
    input.close();

    cout << "Reading obj finished :" << bim_objects.size() << "faces" << endl;
    auto voxel_grid = create_voxel(vertices, 2, 0.3);
    cout << "Finished creating voxel" << voxel_grid.voxels.size() << endl;
    auto intersected_voxel_grid =
        intersection_with_bim_obj(voxel_grid, bim_objects);

    auto marked_voxel_grid = mark_exterior_interior(intersected_voxel_grid);

    extract_surface(marked_voxel_grid);
    // // count the number of classess
    // unsigned int class1 = 0;
    // unsigned int class2 = 0;
    // unsigned int class3 = 0;
    // unsigned int class4 = 0;
    // unsigned int class5 = 0;
    // unsigned int class6 = 0;
    // unsigned int class0 = 0;
    // for (auto x: marked_voxel_grid.voxels) {
    //     for (auto y: x) {
    //         for (auto z: y) {
    //             if (z == 1) {
    //                 class1++;
    //             } else if (z == 2) {
    //                 class2++;
    //             } else if (z == 3) {
    //                 class3++;
    //             } else if (z == 0) {
    //                 class0++;
    //             } else if (z == 4) {
    //                 class4++;
    //             } else if (z == 5) {
    //                 class5++;
    //             } else if (z == 6) {
    //                 class6++;
    //             }
    //         }
    //     }
    // }

    // cout << "class 1: " << class1 << endl;
    // cout << "class 2: " << class2 << endl;
    // cout << "class 3: " << class3 << endl;
    // cout << "class 0: " << class0 << endl;
    // cout << "class 4: " << class4 << endl;
    // cout << "class 5: " << class5 << endl;
    // cout << "class 6: " << class6 << endl;
    // vec<unsigned int> export_classes = {3};
    // std::string export_classes_str;
    // for (const auto &cls : export_classes) {
    //   export_classes_str += std::to_string(cls);
    // }

    string output_file_name = "../../out/voxel.obj";
    // std::string output_file_name =
    //     "../../out/voxel_" + export_classes_str + ".obj";
    bool res = write_voxel_obj(output_file_name, marked_voxel_grid);

    json cj = export_voxel_to_cityjson(marked_voxel_grid);

    write_json(cj, input_output.second);
  }

  //  const char *filename =
  //      (argc > 1) ? argv[1] : "../../input/open_house_ifc4.obj";
  //  std::cout << "Processing: " << filename << std::endl;
  //  std::ifstream input(filename);
  //
  //  BIMObjects bim_obj = read_obj(input);
  //  json outJson =
  //  write_json(outJson, "out.city.json");
  return 0;
}
