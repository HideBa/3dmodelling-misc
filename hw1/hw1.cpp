#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Triangulation_vertex_base_2.h>
#include <CGAL/linear_least_squares_fitting_3.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
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

// const std::string input_file =
//         "/Users/hideba/private-proj/tudelft/third-q/3dmodelling/"
//         "assignments-3dmodelling/hw1/data/NL.IMBAG.Pand.0503100000000010-0.obj";
const std::string input_file =
        "/Users/hideba/private-proj/tudelft/third-q/3dmodelling/"
        "assignments-3dmodelling/hw1/data/NL.IMBAG.Pand.0503100000000138-0.obj";
const std::string output_file =
        "/Users/hideba/private-proj/tudelft/third-q/3dmodelling/"
        "assignments-3dmodelling/hw1/out/test.obj";

typedef Kernel::Point_3 Point3;
typedef Kernel::Point_2 Point2;

struct Vertex {
    double x, y, z;
};

struct Face {
    std::list<int> boundary; // ids of vertices vector
    Kernel::Plane_3 best_plane;
    Triangulation triangulation;
};

std::tuple<std::vector<Vertex>, std::vector<Face>>
readObj(const std::string &input_file) {
    std::vector<Vertex> vertices;
    std::vector<Face> faces;

    // Read file
    std::ifstream input_stream;
    input_stream.open(input_file);
    if (input_stream.is_open()) {
        std::string line;

        // Parse line by line
        while (getline(input_stream, line)) {

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
    return std::tuple<std::vector<Vertex>, std::vector<Face>>(vertices, faces);
}

int main(int argc, const char *argv[]) {

    std::vector<Vertex> vertices;
    std::vector<Face> faces;
    std::tie(vertices, faces) = readObj(input_file);

    // Find best fitting plane of each face
    for (auto &face: faces) {
        std::vector<Kernel::Point_3> points;
        for (int index: face.boundary) {
            const Vertex &v = vertices[index];
            Point3 point = Point3(v.x, v.y, v.z);
            points.push_back(point);
        }

        if (!points.empty()) {
            CGAL::linear_least_squares_fitting_3(points.begin(), points.end(),
                                                 face.best_plane,
                                                 CGAL::Dimension_tag<0>());
        }
    }

    int number_of_faces = 0;
    // Triangulate faces (to do)
    for (auto &face: faces) {
        std::vector<Point2> points;
        for (auto const &vertex_index: face.boundary) {
            Point2 projected_vertex = face.best_plane.to_2d(
                    Point3(vertices[vertex_index].x, vertices[vertex_index].y,
                           vertices[vertex_index].z));
            points.push_back(projected_vertex);
        }
        Triangulation triangulation;
        for (auto const &point: points) {
            triangulation.insert(point);
        }

        triangulation.insert_constraint(points[0], points[1]);
        for (size_t i = 0, next = 1; i < points.size();
             i++, next = (i + 1) % points.size()) {
            triangulation.insert_constraint(points[i], points[next]);
        }

        face.triangulation = triangulation;
        Triangulation::Face_handle infinite_face = triangulation.infinite_face();
        number_of_faces += triangulation.number_of_faces();
    }
    std::cout << "number of finite faces" << number_of_faces << std::endl;

    // Label triangulation (to do)
    // Reference:
    // https://github.com/tudelft3d/prepair/blob/03a6de6a28d9fcabe22f2598e89eb418d7767855/Polygon_repair.h
    for (auto &face: faces) {
        std::list<Triangulation::Face_handle> to_check;
        face.triangulation.infinite_face()->info().processed = true;
        CGAL_assertion(face.triangulation.infinite_face()->info().processed ==
                       true);
        CGAL_assertion(face.triangulation.infinite_face()->info().interior ==
                       false);
        to_check.push_back(face.triangulation.infinite_face());
        while (!to_check.empty()) {
            CGAL_assertion(to_check.front()->info().processed == true);
            for (int neighbour_i = 0; neighbour_i < 3; ++neighbour_i) {
                if (to_check.front()->neighbor(neighbour_i)->info().processed) {

                } else {
                    to_check.front()->neighbor(neighbour_i)->info().processed = true;
                    CGAL_assertion(
                            to_check.front()->neighbor(neighbour_i)->info().processed ==
                            true);
                    if (face.triangulation.is_constrained(
                            Triangulation::Edge(to_check.front(), neighbour_i))) {
                        to_check.front()->neighbor(neighbour_i)->info().interior =
                                !to_check.front()->info().interior;
                        to_check.push_back(to_check.front()->neighbor(neighbour_i));
                    } else {
                        to_check.front()->neighbor(neighbour_i)->info().interior =
                                to_check.front()->info().interior;
                        to_check.push_back(to_check.front()->neighbor(neighbour_i));
                    }
                }
            }
            to_check.pop_front();
        }
    }

    int num_inteiors = 0;
    for (auto &face: faces) {
        Triangulation tris = face.triangulation;
        for (auto face_handle = tris.finite_faces_begin();
             face_handle != tris.finite_faces_end(); ++face_handle) {
            if (face_handle->info().interior) {
                num_inteiors++;
            }
        }
    }
    std::cout << "interior" << num_inteiors << std::endl;

    // Extract vertices and faces from each triangulation
    std::vector<Vertex> vertices_out;
    std::map<Point3, int> vertexIndexMap;
    std::vector<std::array<int, 3>> faces_out;
    int currentIndex = 1;

    for (auto &face: faces) {
        for (auto tri_face = face.triangulation.finite_faces_begin();
             tri_face != face.triangulation.finite_faces_end(); ++tri_face) {
            if (tri_face->info().interior) {
                std::array<int, 3> faceIndices;

                for (int i = 0; i < 3; ++i) {
                    Point3 p = face.best_plane.to_3d(tri_face->vertex(i)->point());
                    // Check if this vertex is already added
                    if (vertexIndexMap.find(p) == vertexIndexMap.end()) {
                        Vertex v{p.x(), p.y(), p.z()};
                        vertices_out.push_back(v);
                        vertexIndexMap[p] = currentIndex++;
                    }
                    faceIndices[i] = vertexIndexMap[p];
                }

                faces_out.push_back(faceIndices);
            }
        }
    }

    // Output to OBJ file
    std::ofstream output_stream(output_file);
    if (output_stream.is_open()) {
        // Write vertices
        for (auto const &vertex: vertices_out) {
            output_stream << "v " << vertex.x << " " << vertex.y << " " << vertex.z
                          << std::endl;
        }
        // Write faces
        for (auto const &face: faces_out) {
            output_stream << "f " << face[0] << " " << face[1] << " " << face[2]
                          << std::endl;
        }
        output_stream.close();
    } else {
        std::cerr << "Could not open output file: " << output_file << std::endl;
    }

    return 0;
}
