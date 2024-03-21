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

json lod1_2(json &j);

json extract_lod1_2(json &j);

vec<double> transform(vec<int> vertices, vec<double> scales, vec<double> translates);

vec<int> rev_transform(vec<double> vertices, vec<double> scales, vec<double> translates);

std::vector<std::string> BUILDING_TYPE = {"Building", "BuildingPart",
                                          "BuildingRoom", "BuildingStorey",
                                          "BuildingUnit"};

int main(int argc, const char *argv[]) {
    //-- will read the file passed as argument or twobuildings.city.json if
    // nothing is passed

    //    const char *filename =
    //            (argc > 1) ? argv[1] : "../../data/tudcampus.city.json";
    //    const char *outFineName =
    //            (argc > 2) ? argv[2] : "../../out/out_tud.city.json";
    const char *filename =
            (argc > 1) ? argv[1] : "../../data/twobuildings.city.json";
    const char *outFineName = (argc > 2) ? argv[2] : "../../out/out.city.json";
    //    const char *filename =
    //            (argc > 1) ? argv[1] : "../../data/specialcase_2.city.json";
    //    const char *outFineName =
    //            (argc > 2) ? argv[2] : "../../out/specialcase_2.city.json";
    //    const char *filename =
    //            (argc > 1) ? argv[1] : "../../data/specialcase_3.city.json";
    //    const char *outFineName =
    //            (argc > 2) ? argv[2] : "../../out/specialcase_3.city.json";
    std::cout << "Processing: " << filename << std::endl;
    std::ifstream input(filename);
    json j;
    input >> j; //-- store the content of the file in a nlohmann::json object
    input.close();

    json lod0_2_json = lod0_2(j);

    json outJson = extract_lod1_2(lod0_2_json);

    //-- write to disk the modified city model (out.city.json)
    std::ofstream o(outFineName);
    o << outJson.dump(2) << std::endl;
//    o << lod0_2_json.dump(2) << std::endl;
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

vec<int> rev_transform(vec<double> vertices, vec<double> scales,
                       vec<double> translates) {
    int v0 = static_cast<int>((vertices[0] - translates[0]) / scales[0]);
    int v1 = static_cast<int>((vertices[1] - translates[1]) / scales[1]);
    int v2 = static_cast<int>((vertices[2] - translates[2]) / scales[2]);
    vec<int> transformed_vertices = {v0, v1, v2};
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
                    pair<int, double> lower_surface;
                    for (size_t i = 0; i < shell.size(); i++) {
                        json surface = shell[i];
                        vec<vec<int>> surface_boundary;
                        for (size_t j = 0; j < surface.size(); j++) {
                            vec<vec<double>> vertices;
                            double total_z = 0;
                            for (auto &boundary: surface[j].get<vec<int>>()) {
                                vec<int> vertex = js["vertices"][boundary];
                                vec<double> scales = js["transform"]["scale"];
                                vec<double> translates = js["transform"]["translate"];
                                vec<double> tv = transform(vertex, scales, translates);
                                vertices.push_back(tv);
                                total_z = total_z + tv[2];
                            };
                            double average_z = total_z / vertices.size();
                            if (average_z < lower_surface.second) {
                                lower_surface = {j, average_z};
                            }
                        }
                    }
                    vec<vec<int>> rev_surface = shell[lower_surface.first];
                    // revese orientation of the inner array
                    for (auto &innerVec: rev_surface) {
                        reverse(innerVec.begin(), innerVec.end());
                    }
                    lod0_2_boundaries.push_back(rev_surface);
                }
            }
            cout << "lod0_2_boundaries size: " << lod0_2_boundaries.size() << endl;
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

            //            extract lod0.2
            json lod1_2_geometry = {{"lod",  "1.2"},
                                    {"type", "MultiSurface"}};
            vec<vec<vec<int>>> lod1_2_boundaries;
            for (auto &g: co.value()["geometry"]) {
                double roof_height;
                if (g["lod"] == "2.2") {
                    for (auto &shell: g["boundaries"]) {
                        // Extract lod0.2
                        vec<vec<vec<int>>> lod0_2_boundaries;
                        pair<int, double> lower_surface;
                        for (size_t i = 0; i < shell.size(); i++) {
                            json surface = shell[i];
                            vec<vec<int>> surface_boundary;
                            for (size_t j = 0; j < surface.size(); j++) {
                                vec<vec<double>> vertices;
                                vec<Point3> points;
                                double total_z = 0;
                                for (auto &boundary: surface[j].get<vec<int>>()) {
                                    vec<int> vertex = js["vertices"][boundary];
                                    vec<double> tv = transform(vertex, scales, translates);
                                    vertices.push_back(tv);
                                    total_z = total_z + tv[2];
                                };
                                double average_z = total_z / vertices.size();
                                if (average_z < lower_surface.second) {
                                    lower_surface = {j, average_z};
                                }
                            }
                        }
                        lod0_2_boundaries.push_back(shell[lower_surface.first]);
                        // Decide the height of the roof
                        for (size_t i = 0; i < shell.size(); i++) {
                            json surface = shell[i];
                            vec<vec<int>> roof_surface_boundary;
                            for (size_t j = 0; j < surface.size(); j++) {
                                vec<vec<double>> vertices;
                                vec<Point3> points;
                                for (auto &boundary: surface[j].get<vec<int>>()) {
                                    vec<int> vertex = js["vertices"][boundary];
                                    vec<double> tv = transform(vertex, scales, translates);
                                    Point3 point = Point3(tv[0], tv[1], tv[2]);
                                    vertices.push_back(tv);
                                    points.push_back(point);
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
                                if (j != 0) { // inner boundary
                                    normal = {-normal[0], -normal[1], -normal[2]};
                                }
                                if (normal[2] > 0) {
                                    roof_surface_boundary.push_back(surface[j]);

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
                                    roof_height = lod1_2_roof_height;
                                }
                            }
                        }

                        vec<pair<int, int>> roof_ground_pairs;
                        // Add roof vertices to vertices list
                        vec<vec<vec<int>>> roof_multi_surface;
                        for (auto &surface: lod0_2_boundaries) {
                            vec<vec<int>> roof_surface;
                            for (auto &boundary: surface) {
                                vec<int> inner_roof;
                                for (auto &v: boundary) {
                                    vec<int> vertex = js["vertices"][v];
                                    vec<double> tv = transform(vertex, scales, translates);

                                    vec<double> roof_v = {tv[0], tv[1], tv[2] + roof_height};
                                    vec<int> rev_vertex = rev_transform(roof_v, scales, translates);
                                    js["vertices"].push_back(rev_vertex);
                                    inner_roof.push_back(js["vertices"].size() - 1);
                                    roof_ground_pairs.push_back({v, js["vertices"].size() - 1});
                                }

                                //reverse the orientation of the inner array
                                reverse(inner_roof.begin(), inner_roof.end());
                                roof_surface.push_back(inner_roof);
                            }
                            roof_multi_surface.push_back(roof_surface);
                        }



//                        MEMO: this is for debug
//                        ========================
                        json lod1_0_geometry = {{"lod",  "1.0"},
                                                {"type", "MultiSurface"}};
                        lod1_0_geometry["boundaries"] = roof_multi_surface;
                        co.value()["geometry"].push_back(lod1_0_geometry);
//                        ========================


                        vec<vec<vec<int>>> multi_ground_surface = lod0_2_boundaries;
                        vec<vec<vec<int>>> lod1_2_boundaries;
                        vec<vec<vec<int>>> wallsurfaces;

                        for (size_t i = 0; i < multi_ground_surface.size(); i++) {
                            vec<vec<int>> ground_surface = multi_ground_surface[i];
                            for (size_t j = 0; j < ground_surface.size(); j++) {
                                vec<int> inner_ground_surface = ground_surface[j];
                                bool is_outer = j == 0;


                                for (size_t k = 0; k < inner_ground_surface.size(); k++) {
                                    vec<vec<int>> wall_surface;
                                    vec<int> inner_wall_surface;

                                    int ground_vertex_1 = inner_ground_surface[k];

                                    int ground_vertex_2 = inner_ground_surface[(k + 1) %
                                                                               inner_ground_surface.size()]; // This is because it should be CCW. In case when it's inner boundary, the orientation should be oppsoite

                                    auto it1 = std::find_if(roof_ground_pairs.begin(),
                                                            roof_ground_pairs.end(),
                                                            [ground_vertex_1](const pair<int, int> &pair) {
                                                                return pair.first == ground_vertex_1;
                                                            });
                                    auto it2 = std::find_if(roof_ground_pairs.begin(),
                                                            roof_ground_pairs.end(),
                                                            [ground_vertex_2](const pair<int, int> &pair) {
                                                                return pair.first == ground_vertex_2;
                                                            });
                                    inner_wall_surface.push_back(ground_vertex_2);
                                    inner_wall_surface.push_back(ground_vertex_1);
                                    inner_wall_surface.push_back(it1->second);
                                    inner_wall_surface.push_back(it2->second);
                                    wall_surface.push_back(inner_wall_surface);
                                    wallsurfaces.push_back(wall_surface);
                                }
                            }
                            wallsurfaces.push_back(roof_multi_surface[i]);
                            lod1_2_boundaries = wallsurfaces;
                        }
                        lod1_2_geometry["boundaries"] = lod1_2_boundaries;
                        co.value()["geometry"].push_back(lod1_2_geometry);

                    }
                }
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