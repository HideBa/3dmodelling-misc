
#include "json.hpp"
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/linear_least_squares_fitting_3.h>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Exact_predicates_tag Tag;

using json = nlohmann::json;
using namespace std;

template <typename T> using vec = std::vector<T>;
typedef Kernel::Point_3 Point3;
typedef Kernel::Tetrahedron_3 Tetra3;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron3;
typedef Polyhedron3::HalfedgeDS HalfedgeDS;
using namespace std;

template <typename T> using vec = std::vector<T>;

Polyhedron3 extrudePolygon(const vec<Point3> &vertices, double height) {
  Polyhedron3 P;
  vec<Point3> roof_vertices;

  for (const auto &v : vertices) {
    roof_vertices.push_back(Point3(v.x(), v.y(), v.z() + height));
  }

  for (size_t i = 0, next = 1; i < vertices.size();
       i++, next = (next + 1) % vertices.size()) {
    vec<Point3> face_vertices = {vertices[i], vertices[next],
                                 roof_vertices[next], roof_vertices[i]};
  }
}