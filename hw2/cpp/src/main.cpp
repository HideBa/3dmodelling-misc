/*
+------------------------------------------------------------------------------+
|                                                                              |
|                                 Hugo Ledoux                                  |
|                             h.ledoux@tudelft.nl                              |
|                                  2024-02-21                                  |
|                                                                              |
+------------------------------------------------------------------------------+
*/

#include <Eigen/Dense>
#include <_types/_intmax_t.h>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

//-- https://github.com/nlohmann/json
//-- used to read and write (City)JSON
#include "json.hpp" //-- it is in the /include/ folder

using json = nlohmann::json;

int get_no_roof_surfaces(json &j);

void list_all_vertices(json &j);

void visit_roofsurfaces(json &j);

json extract_ground_surface(json &j);

int main(int argc, const char *argv[]) {
  //-- will read the file passed as argument or twobuildings.city.json if
  // nothing is passed

  const char *filename =
      (argc > 1) ? argv[1] : "../../data/tudcampus.city.json";
  std::cout << "Processing: " << filename << std::endl;
  //    const char *filename =
  //            (argc > 1) ? argv[1] : "../../data/twobuildings.city.json";
  const char *outFineName =
      (argc > 2) ? argv[2] : "../../out/out_tud.city.json";
  //    const char *outFineName = (argc > 2) ? argv[2] :
  //    "../../out/out.city.json";
  std::ifstream input(filename);
  json j;
  input >> j; //-- store the content of the file in a nlohmann::json object
  input.close();

  //-- get total number of RoofSurface in the file
  //  int noroofsurfaces = get_no_roof_surfaces(j);
  //  std::cout << "Total RoofSurface: " << noroofsurfaces << std::endl;

  //  list_all_vertices(j);
  //
  //  visit_roofsurfaces(j);

  //-- print out the number of Buildings in the file
  //  int nobuildings = 0;
  //  for (auto &co : j["CityObjects"]) {
  //    if (co["type"] == "Building") {
  //      nobuildings += 1;
  //    }
  //  }
  //  std::cout << "There are " << nobuildings << " Buildings in the file"
  //            << std::endl;
  //
  //  //-- print out the number of vertices in the file
  //  std::cout << "Number of vertices " << j["vertices"].size() << std::endl;

  //-- add an attribute "volume"
  //  for (auto &co : j["CityObjects"]) {
  //    if (co["type"] == "Building") {
  //      co["attributes"]["volume"] = rand();
  //    }
  //  }

  json outJson = extract_ground_surface(j);

  //-- write to disk the modified city model (out.city.json)
  std::ofstream o(outFineName);
  o << outJson.dump(2) << std::endl;
  o.close();
  std::cout << "file written" << std::endl;
  return 0;
}

json extract_ground_surface(json &j) {
  std::vector<std::string> buildingType = {"Building", "BuildingPart",
                                           "BuildingRoom", "BuildingStorey",
                                           "BuildingUni"};

  for (auto &co : j["CityObjects"].items()) {
    if (std::find(buildingType.begin(), buildingType.end(),
                  co.value()["type"]) != buildingType.end()) {
      json lod02GeometryObj = {{"lod", "0.2"}, {"type", "MultiSurface"}};
      for (auto &g : co.value()["geometry"]) {
        if (g["type"] != "Solid" || g["lod"] != "2.2") { // TODO: check later
          continue;
        } else {
          for (auto &shell : g["boundaries"]) {
            // TODO: extract boundaries
          }

          // Find index of groundsurface
          json surfaceJ;
          json originalSurface = g["semantics"]["surfaces"];
          // TODO: add error handling
          int groundSurfaceIndex = -1;
          for (size_t i = 0; i < originalSurface.size(); ++i) {
            if (originalSurface[i].contains("type") &&
                originalSurface[i]["type"] == "GroundSurface") {
              groundSurfaceIndex = static_cast<int>(i);
              surfaceJ.push_back(g["semantics"]["surfaces"][i]);
            }
          }
          if (groundSurfaceIndex == -1) {
            std::cout << "no ground surface!!!!!!" << std::endl;
          } else {
            std::cout << "found groudn surface!!!" << std::endl;
          }

          std::cout << "surface :" << &surfaceJ << std::endl;

          json valuesJ =
              g["semantics"]["values"]; // TODO: handle this part later, for
          // MultiPoint and MultiLineString this
          // should be simple array of integer
          std::vector<std::vector<int>> boundaryIndices;
          for (size_t i = 0; i < valuesJ.size(); i++) {
            std::vector<int> indices;
            for (size_t j = 0; j < valuesJ[i].size(); j++) {
              int value = valuesJ[i][j];
              if (value == groundSurfaceIndex) {
                indices.push_back(static_cast<int>(i));
              }
            }
            boundaryIndices.push_back(indices);
          }

          std::vector<int> lod02Values;
          std::vector<std::vector<std::vector<int>>>
              lod02Boundaries; // e.g. [[]]
          for (size_t i = 0; i < boundaryIndices.size(); i++) {
            std::vector<std::vector<int>> boundaries;
            for (size_t j = 0; j < boundaryIndices[i].size(); j++) {
              int boundaryIndex = boundaryIndices[i][j];
              std::vector<int> boundary = g["boundaries"][i][j][boundaryIndex];

              //              MEMO: Flip the boundary so that normal points
              //              upward
              std::reverse(boundary.begin(), boundary.end());

              boundaries.push_back(boundary);
              lod02Values.push_back(static_cast<int>(j));
            }
            lod02Boundaries.push_back(boundaries);
          }
          lod02GeometryObj["semantics"]["values"] = lod02Values;
          lod02GeometryObj["semantics"]["surfaces"] = surfaceJ;
          lod02GeometryObj["boundaries"] = lod02Boundaries;
          json tempJson = lod02Boundaries;
          std::cout << tempJson.dump(2) << std::endl;
          co.value()["geometry"].push_back(lod02GeometryObj);
        }
      }

    } else {
      continue;
    }
  }
  return j;
}