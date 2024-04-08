#ifndef BIM_OBJ_H
#define BIM_OBJ_H

#include "types.h"
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

BIMObjects assign_semantics(BIMObjects &bim_objs) {

  for (auto &bim_obj : bim_objs) {
    auto &shells = bim_obj.second.shells;
    vec<Point3> points;
    for (auto &triangle : shells) {
      points.push_back(triangle.vertex(0));
      points.push_back(triangle.vertex(1));
      points.push_back(triangle.vertex(2));
    }
    if (is_vertical_surface(points)) {
      bim_obj.second.sem = GeometricSemantics::Wall;
    } else {
      double average_height = calc_average_height(points);
      if (average_height > 0) {
        bim_obj.second.sem = GeometricSemantics::Roof;
      } else {
        bim_obj.second.sem = GeometricSemantics::Ground;
      }
    }
  }
  return bim_objs;
}

#endif