/*
+------------------------------------------------------------------------------+
|                                                                              |
|                                 Hugo Ledoux                                  |
|                             h.ledoux@tudelft.nl                              |
|                                  2024-02-21                                  |
|                                                                              |
+------------------------------------------------------------------------------+
*/

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

//-- https://github.com/nlohmann/json
//-- used to read and write (City)JSON
#include "json.hpp" //-- it is in the /include/ folder

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/linear_least_squares_fitting_3.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Exact_predicates_tag Tag;

using json = nlohmann::json;
using namespace std;

template<typename T> using vec = std::vector<T>;
typedef Kernel::Point_2 Point2;
typedef Kernel::Point_3 Point3;
typedef Kernel::Plane_3 Plane3;
typedef CGAL::Polygon_2<Kernel> Plygon2;

json lod0_2(json &j);

json lod1_2(json &j);

vec<double> transform(vec<int> vertices, vec<double> scales,
                      vec<double> translates);

vec<int> rev_transform(vec<double> vertices, vec<double> scales,
                       vec<double> translates);

bool is_vertical_surface(vec<Point3> points);

std::vector<std::string> BUILDING_TYPE = {"Building", "BuildingPart",
                                          "BuildingRoom", "BuildingStorey",
                                          "BuildingUnit"};

bool write_json(const json &j, const std::string &filename) {
    std::ofstream o(filename);
    if (!o.is_open()) {
        return false;
    }
    o << j.dump(2) << std::endl;
    o.close();
    std::cout << "file written" << std::endl;
    return true;
}

int main(int argc, const char *argv[]) {
    vec<pair<string, string>> input_outputs = {

            {"../../data/tudcampus.city.json",
                    "../../out/out_tud.city.json"},
//            {"../../data/twobuildings.city.json",
//                    "../../out/out.city.json"},
            {"../../data/specialcase_1.city.json",
                    "../../out/specialcase_1.city.json"},
            {"../../data/specialcase_2.city.json",
                    "../../out/specialcase_2.city.json"},
            {"../../data/specialcase_3.city.json",
                    "../../out/specialcase_3.city.json"},
            {"../../data/specialcase_4.city.json",
                    "../../out/specialcase_4.city.json"}
    };
    for (auto input_output: input_outputs) {
        std::cout << "Processing: " << input_output.first << std::endl;
        std::ifstream input(input_output.first);
        json j;
        input >> j; //-- store the content of the file in a nlohmann::json object
        input.close();
        json lod0_2_json = lod0_2(j);
        json outJson = lod1_2(lod0_2_json);
        write_json(outJson, input_output.second);
    }

//    const char *filename = (argc > 1) ? argv[1] : "../../data/twobuildings.city.json";
//    std::cout << "Processing: " << filename << std::endl;
//    std::ifstream input(filename);
//    json j;
//    input >> j; //-- store the content of the file in a nlohmann::json object
//    input.close();
//    json lod0_2_json = lod0_2(j);
//    json outJson = lod1_2(lod0_2_json);
//    write_json(outJson, "out.city.json");
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

bool is_vertical_surface(vec<Point3> points) {
    Plane3 plane;
    CGAL::linear_least_squares_fitting_3(points.begin(), points.end(), plane,
                                         CGAL::Dimension_tag<0>());

    auto A = plane.a();
    auto B = plane.b();
    auto C = plane.c();
    const double vertical_tolerance = 0.1;
    bool isVertical =
            std::abs(C) < vertical_tolerance && (std::abs(A) > vertical_tolerance || std::abs(B) > vertical_tolerance);

    return isVertical;
}

// This function averages the height of the points in the surface
double calc_average_height(vec<Point3> points) {
    double total_z = 0;
    for (auto &point: points) {
        total_z = total_z + point.z();
    }
    return total_z / points.size();
}

// This function finds the ground surface of the building. The ground surface is
// the surface with the lowest average height.
vec<vec<int>> find_lowest_surface(const vec<vec<vec<int>>> &shell,
                                  const vec<vec<int>> &cj_vertices,
                                  const vec<double> &cj_scales,
                                  const vec<double> &cj_translates) {
    pair<int, double> lower_surface;
    for (size_t i = 0; i < shell.size(); i++) {
        vec<vec<int>> surface = shell[i];

        for (size_t j = 0; j < surface.size(); j++) {
            vec<Point3> points;
            for (auto &boundary: surface[j]) {
                vec<int> vertex = cj_vertices[boundary];
                vec<double> tv = transform(vertex, cj_scales, cj_translates);
                points.push_back(Point3(tv[0], tv[1], tv[2]));
            };
            double average_height = calc_average_height(points);
            bool is_vertical = is_vertical_surface(points);
            if (!is_vertical) {
                if (lower_surface.second == 0) {
                    lower_surface = {i, average_height};
                }
                if (average_height < lower_surface.second) {
                    lower_surface = {i, average_height};
                }
            }
        }
    }
    vec<vec<int>> ground_surface = shell[lower_surface.first];
    return ground_surface;
}

void removeArrayFromArray(vec<vec<vec<int>>> &array1,
                          const vec<vec<int>> &array2) {
    auto new_end = remove_if(
            array1.begin(), array1.end(),
            [&array2](const vec<vec<int>> &element) { return element == array2; });

    array1.erase(new_end, array1.end());
}

vec<vec<vec<int>>> find_roof_surfaces(const vec<vec<vec<int>>> &shell,
                                      const vec<vec<int>> &cj_vertices,
                                      const vec<double> &cj_scales,
                                      const vec<double> &cj_translates,
                                      const vec<vec<int>> ground_surface) {
    vec<vec<vec<int>>> roof_surfaces;
    for (size_t i = 0; i < shell.size(); i++) {
        vec<vec<int>> surface = shell[i];
        for (size_t j = 0; j < surface.size(); j++) {
            vec<Point3> points;
            for (auto &boundary: surface[j]) {
                vec<int> vertex = cj_vertices[boundary];
                vec<double> tv = transform(vertex, cj_scales, cj_translates);
                points.push_back(Point3(tv[0], tv[1], tv[2]));
            };
            bool is_vertical = is_vertical_surface(points);
            if (!is_vertical) {
                roof_surfaces.push_back(surface);
            }
        }
    }
    removeArrayFromArray(roof_surfaces, ground_surface);
    return roof_surfaces;
}

double calculate_projected_area(const vec<Point3> &points) {
    vec<Point2> projected_points;
    for (auto &point: points) {
        projected_points.push_back(Point2(point.x(), point.y()));
    }
    Plygon2 polygon(projected_points.begin(), projected_points.end());
    return abs(polygon.area());
}

double calc_roof_height(const vec<vec<vec<int>>> &roof_surfaces,
                        const vec<vec<int>> &cj_vertices,
                        const vec<double> &cj_scales,
                        const vec<double> &cj_translates) {
    vec<pair<double, double>> roof_area_height; // area, height
    for (size_t i = 0; i < roof_surfaces.size(); i++) {
        vec<vec<int>> surface = roof_surfaces[i];
        for (size_t j = 0; j < surface.size(); j++) {
            vec<Point3> points;
            for (auto &boundary: surface[j]) {
                vec<int> vertex = cj_vertices[boundary];
                vec<double> tv = transform(vertex, cj_scales, cj_translates);
                points.push_back(Point3(tv[0], tv[1], tv[2]));
            };
            if (!points.empty()) {
                auto minmax_x = std::minmax_element(
                        points.begin(), points.end(),
                        [](const Point3 &a, const Point3 &b) { return a.z() < b.z(); });
                double roof_height = 0;
                if (abs(minmax_x.second->z() - minmax_x.first->z()) < 0.1) {
                    roof_height = minmax_x.second->z();
                } else {
                    roof_height =
                            ((minmax_x.second->z() - minmax_x.first->z()) * 0.7) + minmax_x.first->z();
                }

                double roof_area = calculate_projected_area(points);
                roof_area_height.push_back({roof_area, roof_height});
            }
        }
    }
    // calculate weighted average of roof height
    double total_area = 0;
    for (auto &area_height: roof_area_height) {
        total_area += area_height.first;
    }

    double roof_height = 0;
    for (auto &area_height: roof_area_height) {
        double weight_height = area_height.second * (area_height.first / total_area);
        roof_height += weight_height;
    }
    return roof_height;
}

json lod0_2(json &js) {
    const vec<vec<int>> cj_vertices = js["vertices"];
    const vec<double> cj_translates = js["transform"]["translate"];
    const vec<double> cj_scales = js["transform"]["scale"];
    for (auto &co: js["CityObjects"].items()) {
        if (std::find(BUILDING_TYPE.begin(), BUILDING_TYPE.end(),
                      co.value()["type"]) != BUILDING_TYPE.end()) {
            json lod0_2_geometry = {{"lod",  "0.2"},
                                    {"type", "MultiSurface"}};
            vec<vec<vec<int>>> lod0_2_boundaries;
            for (auto &g: co.value()["geometry"]) {
                if (g["lod"] == "2.2" || "2") {
                    for (auto &shell: g["boundaries"]) {
                        vec<vec<vec<int>>> casted_shell = shell.get<vec<vec<vec<int>>>>();
                        vec<vec<int>> ground_surface = find_lowest_surface(
                                casted_shell, cj_vertices, cj_scales, cj_translates);
                        for (auto &innerVec: ground_surface) {
                            reverse(innerVec.begin(), innerVec.end());
                        }
                        lod0_2_boundaries.push_back(ground_surface);
                    }
                }
            }
            lod0_2_geometry["boundaries"] = lod0_2_boundaries;
            co.value()["geometry"].push_back(lod0_2_geometry);
        }
    }
    return js;
}

json lod1_2(json &js) {
    const vec<double> scales = js["transform"]["scale"];
    const vec<double> translates = js["transform"]["translate"];
    for (auto &co: js["CityObjects"].items()) {
        if (std::find(BUILDING_TYPE.begin(), BUILDING_TYPE.end(),
                      co.value()["type"]) != BUILDING_TYPE.end()) {

            json lod1_2_geometry = {{"lod",  "1.2"},
                                    {"type", "Solid"}};
            for (auto &g: co.value()["geometry"]) {
                if (g["lod"] == "2.2" || "2") {
                    for (auto &shell: g["boundaries"]) {
                        // Extract lod0.2
                        vec<vec<vec<int>>> lod0_2_boundaries;
                        vec<vec<int>> ground_surface =
                                find_lowest_surface(shell.get<vec<vec<vec<int>>>>(),
                                                    js["vertices"], scales, translates);
                        lod0_2_boundaries.push_back(ground_surface);
                        // Extract roof surfaces
                        vec<vec<vec<int>>> roof_surfaces = find_roof_surfaces(
                                shell.get<vec<vec<vec<int>>>>(), js["vertices"], scales,
                                translates, ground_surface);

                        // Calculate roof height
                        double roof_height = calc_roof_height(roof_surfaces, js["vertices"],
                                                              scales, translates);

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
                                    vec<double> roof_v = {tv[0], tv[1], roof_height};
                                    vec<int> rev_vertex =
                                            rev_transform(roof_v, scales, translates);
                                    js["vertices"].push_back(rev_vertex);
                                    inner_roof.push_back(js["vertices"].size() - 1);
                                    roof_ground_pairs.push_back({v, js["vertices"].size() - 1});
                                }
                                // reverse the orientation of the inner array
                                reverse(inner_roof.begin(), inner_roof.end());
                                roof_surface.push_back(inner_roof);
                            }
                            roof_multi_surface.push_back(roof_surface);
                        }

                        vec<vec<vec<vec<int>>>> solid;
                        vec<vec<vec<int>>> multi_ground_surface = lod0_2_boundaries;
                        vec<vec<vec<int>>> lod1_2_boundaries;
                        for (size_t i = 0; i < multi_ground_surface.size(); i++) {
                            vec<vec<vec<int>>> wallsurfaces;
                            vec<vec<int>> ground_surface = multi_ground_surface[i];
                            for (size_t j = 0; j < ground_surface.size(); j++) {
                                vec<int> inner_ground_surface = ground_surface[j];

                                for (size_t k = 0; k < inner_ground_surface.size(); k++) {
                                    vec<vec<int>> wall_surface;
                                    vec<int> inner_wall_surface;

                                    // Construct wall surfaces in CCW order
                                    int ground_vertex_1 = inner_ground_surface[k];

                                    int ground_vertex_2 =
                                            inner_ground_surface[(k + 1) %
                                                                 inner_ground_surface.size()];

                                    auto it1 = std::find_if(
                                            roof_ground_pairs.begin(), roof_ground_pairs.end(),
                                            [ground_vertex_1](const pair<int, int> &pair) {
                                                return pair.first == ground_vertex_1;
                                            });
                                    auto it2 = std::find_if(
                                            roof_ground_pairs.begin(), roof_ground_pairs.end(),
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
                            wallsurfaces.push_back(multi_ground_surface[i]);
                            solid.push_back(wallsurfaces);
                        }
                        lod1_2_geometry["boundaries"] = solid;
                        co.value()["geometry"].push_back(lod1_2_geometry);
                    }
                }
            }
        }
    }
    return js;
}
