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

vec<string> BUILDING_TYPE = {"Building", "BuildingPart", "BuildingRoom",
                             "BuildingStorey", "BuildingUnit"};

vec<string> SEMANTICS = {
    "RoofSurface",         "GroundSurface",     "WallSurface", "ClosureSurface",
    "OuterCeilingSurface", "OuterFloorSurface", "Window",      "Door",
    "InteriorWallSurface", "CeilingSurface",    "FloorSurface"};

#endif