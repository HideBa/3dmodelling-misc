#ifndef CJSON_H
#define CJSON_H

#include "types.h"
#include <cstddef>
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

string semantics_to_string(Semantics semantics) {
  switch (semantics) {
  case Semantics::RoofSurface:
    return "RoofSurface";
  case Semantics::GroundSurface:
    return "GroundSurface";
  case Semantics::WallSurface:
    return "WallSurface";
  case Semantics::ClosureSurface:
    return "ClosureSurface";
  case Semantics::OuterCeilingSurface:
    return "OuterCeilingSurface";
  case Semantics::OuterFloorSurface:
    return "OuterFloorSurface";
  case Semantics::Window:
    return "Window";
  case Semantics::Door:
    return "Door";
  case Semantics::InteriorWallSurface:
    return "InteriorWallSurface";
  case Semantics::CeilingSurface:
    return "CeilingSurface";
  case Semantics::FloorSurface:
    return "FloorSurface";
  default:
    return "UNKOWN";
  }
}

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

  map<string, int> uniqueVertices;
  vec<vec<int>> vertices;
  const string object_name_prefix = "obj";
  map<string, json> semantics_surfaces;
  map<string, json> semantics_values;
  for (unsigned int x = 0; x < vg.voxels.size(); ++x) {
    for (unsigned int y = 0; y < vg.voxels[x].size(); ++y) {
      for (unsigned int z = 0; z < vg.voxels[x][y].size(); ++z) {

        auto voxel_label = vg.voxels[x][y][z].label;
        auto city_object_type = vg.voxels[x][y][z].city_object_type;
        auto room_id = vg.voxels[x][y][z].room_id;

        auto semantics = vg.voxels[x][y][z].semantics;

        if (voxel_label == VoxelLabel::INTERSECTED) {
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
          for (const auto &v : transformed_voxel_vertices) {
            string key =
                to_string(v[0]) + "," + to_string(v[1]) + "," + to_string(v[2]);
            if (uniqueVertices.count(key) == 0) { // Vertex not yet in the map
              vertices.push_back({v[0], v[1], v[2]});
              int new_index = vertices.size() - 1;
              uniqueVertices[key] = new_index;
              vertex_map[&v - &transformed_voxel_vertices[0]] = new_index;
            } else { // Vertex already exists, use its index
              vertex_map[&v - &transformed_voxel_vertices[0]] =
                  uniqueVertices[key];
            }
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

          // check if there is already a city object whose key looks like
          // "obj{voxel_label}". If exist, add the voxel boundary to the
          // existing city object. If not exist, create a new city object.
          string city_object_key;
          if (city_object_type == CityObjectType::BuildingRoom) {
            city_object_key = object_name_prefix + "room" + to_string(room_id);

          } else if (!(city_object_type == CityObjectType::UNKOWN)) {
            city_object_key = object_name_prefix +
                              city_object_type_to_string(city_object_type);
          } else {
            city_object_key = object_name_prefix +
                              city_object_type_to_string(city_object_type);
          }

          if (semantics != Semantics::UNKOWN) {
            // check if semantics already exist
            if (semantics_surfaces.find(city_object_key) ==
                semantics_surfaces.end()) {
              semantics_surfaces[city_object_key] = json::array();
              semantics_values[city_object_key] = json::array();
            }

            string sem_str = semantics_to_string(semantics);

            auto found = find_if(semantics_surfaces[city_object_key].begin(),
                                 semantics_surfaces[city_object_key].end(),
                                 [sem_str](const auto &obj) {
                                   if (obj.contains("type")) {
                                     return obj["type"] == sem_str;
                                   }
                                   return false;
                                 });

            if (found == semantics_surfaces[city_object_key].end()) {
              json sem_obj = json::object();
              sem_obj["type"] = sem_str;
              semantics_surfaces[city_object_key].push_back(sem_obj);
              // This is because CityJSON defines values should be an array of
              // arrays in case "type" is CompositeSolid
              vec<int> inner_values;
              for (int i = 0; i < 6; i++) {
                inner_values.push_back(
                    semantics_surfaces[city_object_key].size() - 1);
              }
              semantics_values[city_object_key].push_back(inner_values);
            } else {
              // add the index of the existing semantics
              vec<int> inner_values;
              for (int i = 0; i < 6; i++) {
                inner_values.push_back(
                    found - semantics_surfaces[city_object_key].begin());
              }
              semantics_values[city_object_key].push_back(inner_values);
            }
          } else {
            json inner_values = json::array();
            for (int i = 0; i < 6; i++) {
              inner_values.push_back(nullptr);
            }
            semantics_values[city_object_key].push_back(inner_values);
          }

          if (j["CityObjects"].find(city_object_key) !=
              j["CityObjects"].end()) {
            j["CityObjects"][city_object_key]["geometry"][0]["boundaries"]
                .push_back(solid_boundary);
          } else {
            json city_object;
            city_object["type"] = city_object_type_to_string(
                city_object_type); // TODO: check later
            json lod3_geometry = json::object();
            lod3_geometry["type"] = "CompositeSolid";
            lod3_geometry["lod"] = "4";
            lod3_geometry["boundaries"] = json::array();
            lod3_geometry["boundaries"].push_back(solid_boundary);
            city_object["geometry"].push_back(lod3_geometry);
            city_object["parents"] = json::array({parent_building_key});
            parent_building["children"].push_back(city_object_key);
            j["CityObjects"][city_object_key] = city_object;
          }
        }
      }
    }
  }
  for (auto &[key, city_object] : j["CityObjects"].items()) {
    city_object["geometry"][0]["semantics"] = json::object();
    city_object["geometry"][0]["semantics"]["values"] = semantics_values[key];
    city_object["geometry"][0]["semantics"]["surfaces"] =
        semantics_surfaces[key];
  }

  j["CityObjects"][parent_building_key] = parent_building;

  j["vertices"] = vertices;

  return j;
}

#endif
