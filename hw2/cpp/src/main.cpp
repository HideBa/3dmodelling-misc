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

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Triangulation_vertex_base_2.h>
#include <CGAL/linear_least_squares_fitting_3.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Exact_predicates_tag Tag;

using json = nlohmann::json;
using namespace std;

template <typename T> using vec = std::vector<T>;
typedef Kernel::Point_3 Point3;

json extract_ground_surface(json &j);

int main(int argc, const char *argv[]) {
  //-- will read the file passed as argument or twobuildings.city.json if
  // nothing is passed

  const char *filename =
      (argc > 1) ? argv[1] : "../../data/tudcampus.city.json";
  const char *outFineName =
      (argc > 2) ? argv[2] : "../../out/out_tud.city.json";
  //    const char *filename =
  //            (argc > 1) ? argv[1] : "../../data/twobuildings.city.json";
  //    const char *outFineName = (argc > 2) ? argv[2] :
  //    "../../out/out.city.json";
  std::cout << "Processing: " << filename << std::endl;
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
  //  std::cout << "Number of vertices " << j["vertices"].size() <<
  //  std::endl;

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

vec<double> transform(vec<double> vertices, vec<double> scales,
                      vec<double> translates) {
  double v0 = vertices[0] * scales[0] + translates[0];
  double v1 = vertices[1] * scales[1] + translates[1];
  double v2 = vertices[2] * scales[2] + translates[2];
  vec<double> transformed_vertices = {v0, v1, v2};
  return transformed_vertices;
};

json extract_ground_surface(json &js) {
  std::vector<std::string> buildingType = {"Building", "BuildingPart",
                                           "BuildingRoom", "BuildingStorey",
                                           "BuildingUnit"};

  for (auto &co : js["CityObjects"].items()) {
    if (std::find(buildingType.begin(), buildingType.end(),
                  co.value()["type"]) != buildingType.end()) {
      json lod02_geometry = {{"lod", "0.2"}, {"type", "MultiSurface"}};
      if (co.key() == "NL.IMBAG.Pand.0503100000030277-0") {
        cout << "same-=------------" << endl;
      }
      for (auto &g : co.value()["geometry"]) {
        vec<vec<vec<int>>> lod02_boundaries;
        for (auto &solid : g["boundaries"]) {
          //                    for (auto &faces: solid) {
          for (size_t i = 0; i < solid.size(); i++) {
            json shell = solid[i];
            vec<vec<int>> surface_boundary;
            //                        for (auto face: shells) {
            for (size_t j = 0; j < shell.size(); j++) {
              vec<int> face = shell[j];
              vec<vec<double>> vertices;
              vec<Point3> points;
              vec<int> face_boundary;
              for (auto &boundary : face) {
                vec<double> vertex = js["vertices"][boundary];
                vec<double> scales = js["transform"]["scale"];
                vec<double> translates = js["transform"]["translate"];
                vec<double> tv = transform(vertex, scales, translates);
                Point3 point = Point3(tv[0], tv[1], tv[2]);
                face_boundary.push_back(static_cast<int>(boundary));
                if (end(points) - begin(points) < 3) {
                  points.push_back(point);
                }
                vertices.push_back(tv);
              };
              Point3 u = Point3(points[1][0] - points[0][0],
                                points[1][1] - points[0][1],
                                points[1][2] - points[0][2]);
              Point3 v = Point3(points[2][0] - points[0][0],
                                points[2][1] - points[0][1],
                                points[2][2] - points[0][2]);
              Point3 normal = {u[1] * v[2] - u[2] * v[1],
                               u[2] * v[0] - u[0] * v[2],
                               u[0] * v[1] - u[1] * v[0]};
              // Ground surface!!!
              if (j != 0) {
                normal = Point3(-1 * normal[0], -1 * normal[1], -1 * normal[2]);
              }
              cout << "normal vector is: " << normal[2] << endl;
              //                            double normal_magnitude =
              //                                    sqrt(normal[0] * normal[0] +
              //                                    normal[1] * normal[1] +
              //                                         normal[2] * normal[2]);
              //                            double cos_theta = normal[2] /
              //                            normal_magnitude; bool is_below =
              //                            cos_theta > sqrt((2)) / 2; cout <<
              //                            "is below?" << is_below << endl;
              if (normal[2] < 0) {
                surface_boundary.push_back(face_boundary);
              }
            }
            lod02_boundaries.push_back(surface_boundary);
          }
        }
        lod02_geometry["boundaries"] = lod02_boundaries;
        co.value()["geometry"].push_back(lod02_geometry);
      }
    }
  }
  return js;
}
// json extract_ground_surface(json &j) {
//  std::vector<std::string> buildingType = {"Building", "BuildingPart",
//                                           "BuildingRoom", "BuildingStorey",
//                                           "BuildingUni"};
//
//  for (auto &co : j["CityObjects"].items()) {
//    if (std::find(buildingType.begin(), buildingType.end(),
//                  co.value()["type"]) != buildingType.end()) {
//      json lod02GeometryObj = {{"lod", "0.2"}, {"type", "MultiSurface"}};
//      for (auto &g : co.value()["geometry"]) {
//        if (g["type"] != "Solid" || g["lod"] != "2.2") { // TODO: check later
//          continue;
//        } else {
//          for (auto &shell : g["boundaries"]) {
//            // TODO: extract boundaries
//          }
//
//          // Find index of groundsurface
//          json surfaceJ;
//          json originalSurface = g["semantics"]["surfaces"];
//          // TODO: add error handling
//          int groundSurfaceIndex = -1;
//          for (size_t i = 0; i < originalSurface.size(); ++i) {
//            if (originalSurface[i].contains("type") &&
//                originalSurface[i]["type"] == "GroundSurface") {
//              groundSurfaceIndex = static_cast<int>(i);
//              surfaceJ.push_back(g["semantics"]["surfaces"][i]);
//            }
//          }
//          if (groundSurfaceIndex == -1) {
//            std::cout << "no ground surface!!!!!!" << std::endl;
//          } else {
//            std::cout << "found groudn surface!!!" << std::endl;
//          }
//
//          std::cout << "surface :" << &surfaceJ << std::endl;
//
//          json valuesJ =
//              g["semantics"]["values"]; // TODO: handle this part later, for
//          // MultiPoint and MultiLineString this
//          // should be simple array of integer
//          std::vector<std::vector<int>> boundaryIndices;
//          for (size_t i = 0; i < valuesJ.size(); i++) {
//            std::vector<int> indices;
//            for (size_t j = 0; j < valuesJ[i].size(); j++) {
//              int value = valuesJ[i][j];
//              if (value == groundSurfaceIndex) {
//                indices.push_back(static_cast<int>(i));
//              }
//            }
//            boundaryIndices.push_back(indices);
//          }
//
//          std::vector<int> lod02Values;
//          std::vector<std::vector<std::vector<int>>>
//              lod02Boundaries; // e.g. [[]]
//          for (size_t i = 0; i < boundaryIndices.size(); i++) {
//            std::vector<std::vector<int>> boundaries;
//            for (size_t j = 0; j < boundaryIndices[i].size(); j++) {
//              int boundaryIndex = boundaryIndices[i][j];
//              std::vector<int> boundary =
//              g["boundaries"][i][j][boundaryIndex];
//
//              //              MEMO: Flip the boundary so that normal points
//              //              upward
//              std::reverse(boundary.begin(), boundary.end());
//
//              boundaries.push_back(boundary);
//              lod02Values.push_back(static_cast<int>(j));
//            }
//
//            lod02Boundaries.push_back(boundaries);
//          }
//          lod02GeometryObj["semantics"]["values"] = lod02Values;
//          lod02GeometryObj["semantics"]["surfaces"] = surfaceJ;
//          lod02GeometryObj["boundaries"] = lod02Boundaries;
//          json tempJson = lod02Boundaries;
//          std::cout << tempJson.dump(2) << std::endl;
//          co.value()["geometry"].push_back(lod02GeometryObj);
//        }
//      }
//
//    } else {
//      continue;
//    }
//  }
//  return j;
//}