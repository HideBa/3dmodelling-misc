#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Triangulation_vertex_base_2.h>
#include <CGAL/linear_least_squares_fitting_3.h>

// typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Simple_cartesian<double> Kernel;
typedef CGAL::Exact_predicates_tag Tag;

struct FaceInfo {
  bool interior, processed;
  FaceInfo() {
    processed = false;
    interior = false;
  }
};
typedef CGAL::Triangulation_vertex_base_2<Kernel> VertexBase;
typedef CGAL::Constrained_triangulation_face_base_2<Kernel> FaceBase;
typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo, Kernel, FaceBase>
    FaceBaseWithInfo;
typedef CGAL::Triangulation_data_structure_2<VertexBase, FaceBaseWithInfo>
    TriangulationDataStructure;
typedef CGAL::Constrained_Delaunay_triangulation_2<
    Kernel, TriangulationDataStructure, Tag>
    Triangulation;

const std::string input_file = "./data/NL.IMBAG.Pand.0503100000000010-0.obj";
const std::string output_file = "./out/test.obj";

// TODO: delete me later
typedef Kernel::Point_3 Point;
typedef Kernel::Plane_3 Plane;
typedef CGAL::Simple_cartesian<double>::Triangle_3 Triangle;

struct Vertex {
  double x, y, z;
};

struct Face {
  std::list<int> boundary; // ids of vertices vector
  Kernel::Plane_3 best_plane;
  Triangulation triangulation;
};

int main(int argc, const char *argv[]) {

  std::vector<Vertex> vertices;
  std::vector<Face> faces;

  // Read file
  std::ifstream input_stream;
  input_stream.open(input_file);
  if (input_stream.is_open()) {
    std::string line;

    // Parse line by line
    while (getline(input_stream, line)) {
      std::cout << line << std::endl;

      std::istringstream line_stream(line);
      char line_type;
      line_stream >> line_type;

      // Vertex
      if (line_type == 'v') {
        vertices.emplace_back();
        double x, y, z;
        line_stream >> x >> y >> z;
        vertices.back().x = x;
        vertices.back().y = y;
        vertices.back().z = z;
      }

      // Face
      if (line_type == 'f') {
        faces.emplace_back();
        int v;
        while (!line_stream.eof()) {
          line_stream >> v;
          faces.back().boundary.emplace_back(v - 1);
        }
      }
    }
  }

  // Print vertices
  // int i = 0;
  // for (auto const &vertex : vertices) {
  //   std::cout << "Vertex " << i++ << ": "
  //             << "(" << vertex.x << ", " << vertex.y << ", " << vertex.z
  //             <<
  //             ")"
  //             << std::endl;
  // }

  // // Print faces
  // i = 0;
  // for (auto const &face : faces) {
  //   std::cout << "Face " << i++ << ": ";
  //   for (auto const &vertex : face.boundary)
  //     std::cout << " " << vertex;
  //   std::cout << std::endl;
  // }

  for (auto const &face : faces) {
    std::cout << "best fitting__2222oo____" << std::endl;
    std::vector<Kernel::Point_3> points;

    for (int index : face.boundary) {
      const Vertex &v = vertices[index];
      points.push_back(Kernel::Point_3(v.x, v.y, v.z));
    }

    if (!points.empty()) {
      Plane plane;
      CGAL::linear_least_squares_fitting_3(points.begin(), points.end(), plane,
                                           CGAL::Dimension_tag<2>());
    }
  }

  // Triangulate faces (to do)
  // Project to 2D plane
  // for (auto const &face : faces) {
  //   std::cout << "best fitting!!!!!!!" << std::endl;
  //   for (auto const &vertex_index : face.boundary) {
  //     CGAL::Point_2<Kernel> projected_vertex;
  //     projected_vertex = face.best_plane.to_2d(CGAL::Point_3<Kernel>(
  //         vertices[vertex_index].x, vertices[vertex_index].y,
  //         vertices[vertex_index].z));
  //   }
  //   std::cout << "best plane" << i++ << ": " << face.best_plane;
  //   std::cout << std::endl;
  // }
  // Label triangulation (to do)

  // Export triangles (to do)

  return 0;
}