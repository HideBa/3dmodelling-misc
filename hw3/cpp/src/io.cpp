#ifndef IO_H
#define IO_H

#include "types.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

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

BIMObject::BIMObject(string name, vec<Triangle3> shells)
    : name(move(name)), shells(move(shells)) {}

BIMObject::BIMObject() = default;

pair<BIMObjects, vec<vec<double>>> read_obj(ifstream &input_stream) {
  BIMObjects bim_objects;
  vec<vec<double>> vertices;
  vec<Triangle3> shells;

  if (input_stream.is_open()) {
    std::string line;
    BIMObject bim_obj;
    // Parse line by line
    while (getline(input_stream, line)) {

      istringstream line_stream(line);
      string line_type;
      line_stream >> line_type;

      if (line_type == "g") {
        if (!bim_obj.name.empty()) {
          bim_objects[bim_obj.name] = bim_obj;
          bim_obj = BIMObject();
        }
        line_stream >> bim_obj.name;
      } else if (line_type == "v") {
        vec<double> vertex(3);
        line_stream >> vertex[0] >> vertex[1] >> vertex[2];
        vertices.push_back(vertex);
      } else if (line_type == "f") {
        vec<Point3> points;
        Triangle3 triangle;
        string vertex;
        while (line_stream >> vertex) {
          istringstream vertex_stream(vertex);
          int vi;
          vertex_stream >> vi;
          vec<double> v = vertices[vi - 1];

          points.emplace_back(vertices[vi - 1][0], vertices[vi - 1][1],
                              vertices[vi - 1][2]);
        }
        triangle = Triangle3(points.at(0), points.at(1), points.at(2));
        bim_obj.shells.push_back(triangle);
      }
    }
    if (!bim_obj.name.empty()) {
      bim_objects[bim_obj.name] = bim_obj;
    }
  }
  return make_pair(bim_objects, vertices);
}

// This is for debugging purposes
int write_voxel_obj(const string &outfile, const VoxelGrid &vg,
                    vec<VoxelLabel> export_labels = {VoxelLabel::INTERSECTED}) {
  ofstream outFile(outfile);
  if (!outFile.is_open()) {
    cerr << "Failed to open " << outfile << endl;
    return 0;
  }
  cout << "Writing to " << outfile << endl;
  unsigned int vertex_count = 0;
  for (unsigned int x = 0; x < vg.voxels.size(); x++) {
    for (unsigned int y = 0; y < vg.voxels[x].size(); y++) {
      for (unsigned int z = 0; z < vg.voxels[x][y].size(); z++) {
        if (find(export_labels.begin(), export_labels.end(),
                 vg.voxels[x][y][z].label) != export_labels.end()) {
          double min_x = vg.offset_origin[0] + x * vg.resolution;
          double min_y = vg.offset_origin[1] + y * vg.resolution;
          double min_z = vg.offset_origin[2] + z * vg.resolution;
          double max_x = min_x + vg.resolution;
          double max_y = min_y + vg.resolution;
          double max_z = min_z + vg.resolution;

          outFile << "v " << min_x << " " << min_y << " " << min_z << "\n";
          outFile << "v " << max_x << " " << min_y << " " << min_z << "\n";
          outFile << "v " << min_x << " " << max_y << " " << min_z << "\n";
          outFile << "v " << max_x << " " << max_y << " " << min_z << "\n";
          outFile << "v " << min_x << " " << min_y << " " << max_z << "\n";
          outFile << "v " << max_x << " " << min_y << " " << max_z << "\n";
          outFile << "v " << min_x << " " << max_y << " " << max_z << "\n";
          outFile << "v " << max_x << " " << max_y << " " << max_z << "\n";

          unsigned int v1 = vertex_count + 1;
          outFile << "f " << v1 + 0 << " " << v1 + 1 << " " << v1 + 3 << " "
                  << v1 + 2 << "\n";
          outFile << "f " << v1 + 4 << " " << v1 + 5 << " " << v1 + 7 << " "
                  << v1 + 6 << "\n";
          outFile << "f " << v1 + 0 << " " << v1 + 1 << " " << v1 + 5 << " "
                  << v1 + 4 << "\n";
          outFile << "f " << v1 + 2 << " " << v1 + 3 << " " << v1 + 7 << " "
                  << v1 + 6 << "\n";
          outFile << "f " << v1 + 1 << " " << v1 + 3 << " " << v1 + 7 << " "
                  << v1 + 5 << "\n";
          outFile << "f " << v1 + 0 << " " << v1 + 2 << " " << v1 + 6 << " "
                  << v1 + 4 << "\n";

          vertex_count += 8;
        }
      }
    }
  }
  outFile.close();
  cout << "File has been written " << outfile << endl;
  return 1;
}

#endif