
#include "io.cpp"
#include "types.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
using namespace std;

std::vector<std::string> BUILDING_TYPE = {"Building", "BuildingPart",
                                          "BuildingRoom", "BuildingStorey",
                                          "BuildingUnit"};

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
    auto voxel_grid = create_voxel(vertices);
    cout << "Finished creating voxel" << voxel_grid.voxels.size() << endl;
    auto marked_voxel_grid =
        intersection_with_bim_obj(voxel_grid, bim_objects, 0.5);

    bool res = write_voxel_obj("../../out/voxel.obj", marked_voxel_grid);

    // json outJson =
    // write_json(outJson, input_output.second);
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
