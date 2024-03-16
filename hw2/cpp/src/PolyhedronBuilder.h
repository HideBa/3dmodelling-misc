#ifndef PolyhedronBuilder_H
#define PolyhedronBuilder_H
#include <_types/_intmax_t.h>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "json.hpp" //-- it is in the /include/ folder

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Triangulation_vertex_base_2.h>
#include <CGAL/linear_least_squares_fitting_3.h>

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

// Reference: https://doc.cgal.org/latest/Polyhedron/index.html#title3
template <class HalfedgeDS>
class PolyhedronBuilder : public CGAL::Modifier_base<HalfedgeDS> {
public:
  vec<vec<Point3>> shells;
  PolyhedronBuilder(const vec<vec<Point3>> &shells) : shells(shells) {}
  void operator()(HalfedgeDS &hds) {
    CGAL::Polyhedron_incremental_builder_3<HalfedgeDS> builder(hds, true);
    builder.begin_surface(3, 1, 6);
    // typedef typename HalfedgeDS::Vertex Vertex;
    // typedef typename Vertex::Point Point;
    for (const auto &shell : shells) {
      vec<size_t> vertexIndices;
      for (const auto &point : shell) {
        vertexIndices.push_back(builder.add_vertex(point));
      }
      builder.begin_facet();
      for (auto i : vertexIndices) {
        builder.add_vertex_to_facet(i);
      }
      builder.end_facet();
    }

    if (builder.check_unconnected_vertices()) {
      builder.remove_unconnected_vertices();
    }
    builder.end_surface();
  }
};

// int main() {
//   Polyhedron P;
//   Builder<HalfedgeDS> triangle;
//   P.delegate(triangle);
//   assert(P.is_triangle(P.halfedges_begin()));
//   return 0;
// }

#endif