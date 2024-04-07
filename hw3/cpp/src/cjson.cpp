#ifndef CJSON_H
#define CJSON_H

#include "types.h"
#include <vector>

using namespace std;
template <typename T> using vec = std::vector<T>;

vec<double> transform(vec<int> vertices, vec<double> scales,
                      vec<double> translates);

vec<int> rev_transform(vec<double> vertices, vec<double> scales,
                       vec<double> translates);

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

string city_object_type_to_string(CityObjectType type) {
  switch (type) {
  case CityObjectType::Building:
    return "Building";
  case CityObjectType::BuildingPart:
    return "BuildingPart";
  case CityObjectType::BuildingRoom:
    return "BuildingRoom";
  case CityObjectType::BuildingStorey:
    return "BuildingStorey";
  case CityObjectType::BuildingUnit:
    return "BuildingUnit";
  case CityObjectType::BuildingInstallation:
    return "BuildingInstallation";
  case CityObjectType::BuildingConstructiveElement:
    return "BuildingConstructiveElement";
  case CityObjectType::BuildingFurniture:
    return "BuildingFurniture";
  default:
    return "UNKOWN";
  }
}

vec<string> SEMANTICS = {
    "RoofSurface",         "GroundSurface",     "WallSurface", "ClosureSurface",
    "OuterCeilingSurface", "OuterFloorSurface", "Window",      "Door",
    "InteriorWallSurface", "CeilingSurface",    "FloorSurface"};

json export_voxel_to_cityjson(VoxelGrid &vg) {
  const vec<double> scale = {0.001, 0.001, 0.001};
  const vec<double> translate = {0, 0, 0};
  json j;
  j["type"] = "CityJSON";
  j["version"] = "2.0";
  j["CityObjects"] = json::object();
  j["vertices"] = json::array();
  j["transform"] = json::object();
  j["transform"]["scale"] = scale;
  j["transform"]["translate"] = translate;
  j["metadata"] = json::object();
  j["metadata"]["identifier"] = "hw3";
  j["extensions"] = json::object();

  // add parent building
  json parent_building;
  parent_building["type"] =
      city_object_type_to_string(CityObjectType::Building);
  parent_building["geometry"] = json::array();
  parent_building["children"] = json::array();
  const string parent_building_key = "obj_parent_building";

  vec<vec<int>> vertices;
  const string object_name_prefix = "obj";
  for (unsigned int x = 0; x < vg.voxels.size(); ++x) {
    for (unsigned int y = 0; y < vg.voxels[x].size(); ++y) {
      for (unsigned int z = 0; z < vg.voxels[x][y].size(); ++z) {
        auto voxel_label = vg.voxels[x][y][z].label;
        auto city_object_type = vg.voxels[x][y][z].city_object_type;
        auto room_id = vg.voxels[x][y][z].room_id;
        if (voxel_label ==
            VoxelLabel::INTERSECTED) { // TODO: check what happen when accessing
                                       // like vg(x, y, z)
                                       //             if ((voxel_label != 0) &&
                                       //            (voxel_label !=
          //             2)) { // TODO: check what happen when accessing like
          //             vg(x, y, z)
          double min_x = vg.offset_origin[0] + x * vg.resolution;
          double min_y = vg.offset_origin[1] + y * vg.resolution;
          double min_z = vg.offset_origin[2] + z * vg.resolution;
          double max_x = min_x + vg.resolution;
          double max_y = min_y + vg.resolution;
          double max_z = min_z + vg.resolution;
          vec<vec<double>> voxel_vertices = {
              {min_x, min_y, min_z}, {max_x, min_y, min_z},
              {min_x, max_y, min_z}, {max_x, max_y, min_z},
              {min_x, min_y, max_z}, {max_x, min_y, max_z},
              {min_x, max_y, max_z}, {max_x, max_y, max_z}};

          vec<vec<int>> transformed_voxel_vertices = {
              rev_transform(voxel_vertices[0], scale, translate),
              rev_transform(voxel_vertices[1], scale, translate),
              rev_transform(voxel_vertices[2], scale, translate),
              rev_transform(voxel_vertices[3], scale, translate),
              rev_transform(voxel_vertices[4], scale, translate),
              rev_transform(voxel_vertices[5], scale, translate),
              rev_transform(voxel_vertices[6], scale, translate),
              rev_transform(voxel_vertices[7], scale, translate)};
          // indices to vertices. Orientation is CCW so that normal vector of
          // cube direct outward
          vec<vec<int>> voxel_faces = {{0, 2, 3, 1}, {4, 5, 7, 6},
                                       {0, 1, 5, 4}, {2, 6, 7, 3},
                                       {1, 3, 7, 5}, {0, 4, 6, 2}};
          map<int, int> vertex_map; // {old_index: new_index}
          for (int i = 0; i < transformed_voxel_vertices.size(); ++i) {
            vec<int> v = transformed_voxel_vertices[i];
            vertices.push_back(v);
            vertex_map[i] = vertices.size() - 1;
          }

          vec<vec<vec<vec<int>>>> solid_boundary;
          vec<vec<vec<int>>> outer_shell_boundary;

          for (const auto &face : voxel_faces) {
            vec<vec<int>> face_boundary;
            vec<int> outer_face_indices;
            for (const auto &vertex_index : face) {
              outer_face_indices.push_back(vertex_map[vertex_index]);
            }
            // inner face doesn't exist as it's voxel
            face_boundary.push_back(outer_face_indices);
            outer_shell_boundary.push_back(face_boundary);
          }
          solid_boundary.push_back(
              outer_shell_boundary); // inner shell doesn't exist as it's voxel

          if (voxel_label == VoxelLabel::INTERSECTED) {
            // check if there is already a city object whose key looks like
            // "obj{voxel_label}". If exist, add the voxel boundary to the
            // existing city object. If not exist, create a new city object.
            string city_object_key = object_name_prefix + to_string(room_id);
            if (j["CityObjects"].find(city_object_key) !=
                j["CityObjects"].end()) {
              j["CityObjects"][city_object_key]["geometry"][0]["boundaries"]
                  .push_back(solid_boundary);
              // TODO: //add semantics as well
            } else {
              json city_object;
              city_object["type"] = city_object_type_to_string(
                  city_object_type); // TODO: check later
              json lod3_geometry = json::object();
              lod3_geometry["type"] = "CompositeSolid";
              lod3_geometry["lod"] = "3";
              lod3_geometry["boundaries"] = json::array();
              lod3_geometry["boundaries"].push_back(solid_boundary);
              city_object["geometry"].push_back(lod3_geometry);
              city_object["parents"] = json::array({parent_building_key});
              parent_building["children"].push_back(city_object_key);
              // city_object["geometry"]["semantics"] = json::object();
              // city_object["geometry"]["semantics"]["values"] = json::array();
              // city_object["geometry"]["semantics"]["surfaces"] =
              // json::array();
              // city_object["geometry"]["semantics"]["surfaces"].push_back(
              //     SEMANTICS[voxel_label - 1]);
              // city_object["geometry"]["semantics"]["values"].push_back(1);
              j["CityObjects"][city_object_key] = city_object;
            }
          }
        }
      }
    }
  }
  j["CityObjects"][parent_building_key] = parent_building;

  j["vertices"] = vertices;

  return j;
}

#endif
