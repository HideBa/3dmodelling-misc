#ifndef BIM_OBJ_H
#define BIM_OBJ_H

#include "types.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

bool is_vertical_surface(vec<Point3> points) {
  Plane3 plane;
  CGAL::linear_least_squares_fitting_3(points.begin(), points.end(), plane,
                                       CGAL::Dimension_tag<0>());

  auto A = plane.a();
  auto B = plane.b();
  auto C = plane.c();
  const double vertical_tolerance = 0.1;
  bool isVertical =
      std::abs(C) < vertical_tolerance &&
      (std::abs(A) > vertical_tolerance || std::abs(B) > vertical_tolerance);

  return isVertical;
}

// This function averages the height of the points in the surface
double calc_average_height(vec<Point3> points) {
  double total_z = 0;
  for (auto &point : points) {
    total_z = total_z + point.z();
  }
  return total_z / points.size();
}

string to_lower_case(const string &str) {
  string result = str;
  transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

bool containsSubstring(const string &str, const string &substr) {
  string strLower = to_lower_case(str);
  string substrLower = to_lower_case(substr);
  return strLower.find(substrLower) != string::npos;
}

void assign_semantics(BIMObjects &bim_objs) {
  // check if object name contains "wall", "floor", "wall:interior",
  // "window", "door". If these are contained in the name, assign the
  // corresponding semantics
  for (auto &bim_obj : bim_objs) {
    if (containsSubstring(bim_obj.first, "wall:interior")) {
      bim_obj.second.sem = GeometricSemantics::InteriorWall;
    } else if (containsSubstring(bim_obj.first, "floor") ||
               containsSubstring(bim_obj.first, "footing")) {
      bim_obj.second.sem = GeometricSemantics::Floor;
    } else if (containsSubstring(bim_obj.first, "roof")) {
      bim_obj.second.sem = GeometricSemantics::Roof;
    } else if (containsSubstring(bim_obj.first, "wall")) {
      bim_obj.second.sem = GeometricSemantics::Wall;
    } else if (containsSubstring(bim_obj.first, "window")) {
      bim_obj.second.sem = GeometricSemantics::Window;
    } else if (containsSubstring(bim_obj.first, "door")) {
      bim_obj.second.sem = GeometricSemantics::Door;
    } else {
      bim_obj.second.sem = GeometricSemantics::Other;
    }
  }
}

#endif
