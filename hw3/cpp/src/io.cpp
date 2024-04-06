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

#endif