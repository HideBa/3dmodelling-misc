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
#include "PolyhedronBuilder.h"
#include "json.hpp" //-- it is in the /include/ folder

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Triangulation_vertex_base_2.h>
#include <CGAL/draw_polyhedron.h>
#include <CGAL/linear_least_squares_fitting_3.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Exact_predicates_tag Tag;

using json = nlohmann::json;
using namespace std;

template<typename T> using vec = std::vector<T>;
typedef Kernel::Point_3 Point3;
typedef Kernel::Tetrahedron_3 Tetra3;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron3;
typedef Polyhedron3::HalfedgeDS HalfedgeDS;

json extract_lod0_2(json &j);

json lod0_2(json &j);

json extract_lod1_2(json &j);

std::vector<std::string> BUILDING_TYPE = {"Building", "BuildingPart",
                                          "BuildingRoom", "BuildingStorey",
                                          "BuildingUnit"};

int main(int argc, const char *argv[]) {
    //-- will read the file passed as argument or twobuildings.city.json if
    // nothing is passed

    const char *filename =
            (argc > 1) ? argv[1] : "../../data/tudcampus.city.json";
    const char *outFineName =
            (argc > 2) ? argv[2] : "../../out/out_tud.city.json";
//    const char *filename =
//            (argc > 1) ? argv[1] : "../../data/twobuildings.city.json";
//    const char *outFineName = (argc > 2) ? argv[?2] : "../../out/out.city.json";
    std::cout << "Processing: " << filename << std::endl;
    std::ifstream input(filename);
    json j;
    input >> j; //-- store the content of the file in a nlohmann::json object
    input.close();

    json lod0_2_json = lod0_2(j);
//    json lod0_2_json = extract_lod0_2(j);

//    json outJson = extract_lod1_2(lod0_2_json);

    //-- write to disk the modified city model (out.city.json)
    std::ofstream o(outFineName);
//    o << outJson.dump(2) << std::endl;
    o << lod0_2_json.dump(2) << std::endl;
    o.close();
    std::cout << "file written" << std::endl;
    return 0;
}

vec<double> transform(vec<int> vertices, vec<double> scales,
                      vec<double> translates) {
    double v0 = vertices[0] * scales[0] + translates[0];
    double v1 = vertices[1] * scales[1] + translates[1];
    double v2 = vertices[2] * scales[2] + translates[2];
    vec<double> transformed_vertices = {v0, v1, v2};
    return transformed_vertices;
};

json lod0_2(json &js) {
    for (auto &co: js["CityObjects"].items()) {
        if (std::find(BUILDING_TYPE.begin(), BUILDING_TYPE.end(),
                      co.value()["type"]) != BUILDING_TYPE.end()) {
            json lod0_2_geometry = {{"lod",  "0.2"},
                                    {"type", "MultiSurface"}};
            vec<vec<vec<int>>> lod0_2_boundaries;
            for (auto &g: co.value()["geometry"]) {
                for (auto &shell: g["boundaries"]) {
                    for (size_t i = 0; i < shell.size(); i++) {
                        pair<int, double> lower_surface;
                        json surface = shell[i];
                        vec<vec<int>> surface_boundary;
                        for (size_t j = 0; j < surface.size(); j++) {
                            vec<vec<double>> vertices;
                            vec<Point3> points;
                            double total_z = 0;
                            for (auto &boundary: surface[j].get<vec<int>>()) {
                                vec<int> vertex = js["vertices"][boundary];
                                vec<double> scales = js["transform"]["scale"];
                                vec<double> translates = js["transform"]["translate"];
                                vec<double> tv = transform(vertex, scales, translates);
                                Point3 point = Point3(tv[0], tv[1], tv[2]);
                                vertices.push_back(tv);
                                total_z = total_z + tv[2];
                            };
                            double average_z = total_z / vertices.size();
                            if (average_z < lower_surface.second) {
                                lower_surface = {j, average_z};
                            }

                        }
                        lod0_2_boundaries.push_back(shell[lower_surface.first]);
                    }
                }
            }
            lod0_2_geometry["boundaries"] = lod0_2_boundaries;
            co.value()["geometry"].push_back(lod0_2_geometry);
        }
    }
    return js;
}


json extract_lod1_2(json &js) {
    const vec<double> scales = js["transform"]["scale"];
    const vec<double> translates = js["transform"]["translate"];
    for (auto &co: js["CityObjects"].items()) {
        if (std::find(BUILDING_TYPE.begin(), BUILDING_TYPE.end(),
                      co.value()["type"]) != BUILDING_TYPE.end()) {
            json lod1_2_geometry = {{"lod",  "0.7"},
                                    {"type", "MultiSurface"}};
            for (auto &g: co.value()["geometry"]) {
                if (g["lod"] == "2.2") {
                    vec<vec<vec<int>>> lod1_2_boundaries;
                    for (auto &shell: g["boundaries"]) {
                        for (size_t i = 0; i < shell.size(); i++) {
                            json surface = shell[i];
                            vec<vec<int>> surface_boundary;
                            for (size_t j = 0; j < surface.size(); j++) {
                                vec<vec<double>> vertices;
                                vec<Point3> points;
                                vec<int> face_boundary;
                                for (auto &boundary: surface[j].get<vec<int>>()) {
                                    vec<int> vertex = js["vertices"][boundary];
                                    vec<double> tv = transform(vertex, scales, translates);
                                    Point3 point = Point3(tv[0], tv[1], tv[2]);
                                    face_boundary.push_back(boundary);
                                    points.push_back(point);
                                    vertices.push_back(tv);
                                };
                                if (end(points) - begin(points) < 3) {
                                    cout << "Warning: less than 3 points in a face!";
                                }
                                Point3 u = Point3(points[1][0] - points[0][0],
                                                  points[1][1] - points[0][1],
                                                  points[1][2] - points[0][2]);
                                Point3 v = Point3(points[2][0] - points[0][0],
                                                  points[2][1] - points[0][1],
                                                  points[2][2] - points[0][2]);
                                Point3 normal = {u[1] * v[2] - u[2] * v[1],
                                                 u[2] * v[0] - u[0] * v[2],
                                                 u[0] * v[1] - u[1] * v[0]};
                                if (normal[2] > 0) {
                                    surface_boundary.push_back(face_boundary);
                                    //                                    cout << "normal vector
                                    //                                    is: " << normal[2] <<
                                    //                                    endl;
                                    // find highest and lowest vertex
                                    double lod1_2_roof_height;
                                    if (!points.empty()) {
                                        auto minmax = std::minmax_element(
                                                points.begin(), points.end(),
                                                [](const auto &lhs, const auto &rhs) {
                                                    return lhs[2] < rhs[2];
                                                });
                                        lod1_2_roof_height =
                                                (((*minmax.second)[2] - (*minmax.first)[2]) * 0.7) +
                                                (*minmax.first)[2];
                                        cout << "roof height is: " << lod1_2_roof_height << endl;
                                    }
                                }
                            }
                            lod1_2_boundaries.push_back(surface_boundary);
                        }
                    }
                    lod1_2_geometry["boundaries"] = lod1_2_boundaries;
                    co.value()["geometry"].push_back(lod1_2_geometry);
                }
            }
        }
    }
    return js;
}

// The way using volume
// json extract_lod1_2(json &js) {
//   const vec<double> scales = js["transform"]["scale"];
//   const vec<double> translates = js["transform"]["translate"];
//   for (auto &co : js["CityObjects"].items()) {
//     if (std::find(BUILDING_TYPE.begin(), BUILDING_TYPE.end(),
//                   co.value()["type"]) != BUILDING_TYPE.end()) {
//       json lod1_2_geometry = {{"lod", "1.2"}, {"type", "Solid"}};
//       for (auto &g : co.value()["geometry"]) {
//         double lod2_2_volume;
//         if (g["lod"] == "2.2") {
//           vec<vec<vec<vec<Point3>>>> shells_v;
//           for (auto &shell : g["boundaries"]) {
//             vec<vec<vec<Point3>>> shell_v;
//             for (auto &surface : shell) {
//               vec<vec<Point3>> surface_v;
//               for (auto &ring : surface) {
//                 vec<Point3> ring_v;
//                 for (auto &v : ring) {
//                   vec<int> vi = js["vertices"][v.get<int>()];
//                   vec<double> tv = transform(vi, scales, translates);
//                   Point3 point = Point3(tv[0], tv[1], tv[2]);
//                   ring_v.push_back(point);
//                 }
//                 surface_v.push_back(ring_v);
//               }
//               Polyhedron3 polyhedron;
//               PolyhedronBuilder<HalfedgeDS> builder(surface_v);
//               polyhedron.delegate(builder);
//               double vol =
//               CGAL::Polygon_mesh_processing::volume(polyhedron);
//               shell_v.push_back(surface_v);
//             }
//             shells_v.push_back(shell_v);
//           }
//         }
//       }
//     }
//   }
// }
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
//        if (g["type"] != "Solid" || g["lod"] != "2.2") { // TODO: check
//        later
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