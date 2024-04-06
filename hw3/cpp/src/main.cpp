
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
    map<string, BIMObject> bim_objects = read_obj(input);
    input.close();

    for (auto const bim_object : bim_objects) {
      std::cout << "Processing: " << bim_object.first << std::endl;
      BIMObject bim_obj = bim_object.second;
      vec<Triangle3> shells = bim_obj.shells;
      cout << "size of shells: " << shells.size()
           << "\n"; // << "size of vertices: " << vertices.size() << "\n";
      for (auto const shell : shells) {
        std::cout << "Processing: " << shell << std::endl;
      }
    }
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
