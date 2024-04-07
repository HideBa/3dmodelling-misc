#ifndef TYPES_H
#define TYPES_H

#include <fstream>
#include <map>
#include <sstream>
#include <string>
//-- https://github.com/nlohmann/json
//-- used to read and write (City)JSON
#include "json.hpp" //-- it is in the /include/ folder

#include "CGAL/Bbox_3.h"
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/linear_least_squares_fitting_3.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Exact_predicates_tag Tag;

using json = nlohmann::json;
using namespace std;

template <typename T> using vec = std::vector<T>;
typedef Kernel::Point_2 Point2;
typedef Kernel::Point_3 Point3;
typedef Kernel::Plane_3 Plane3;
typedef CGAL::Polygon_2<Kernel> Plygon2;
typedef Kernel::Triangle_3 Triangle3;
typedef CGAL::Bbox_3 Bbox3;
typedef CGAL::Triangulation_vertex_base_2<Kernel> VertexBase;
typedef CGAL::Constrained_triangulation_face_base_2<Kernel> FaceBase;
// typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo, Kernel, FaceBase>
// FaceBaseWithInfo;
typedef CGAL::Triangulation_data_structure_2<VertexBase>
    TriangulationDataStructure;

// BIM Object
struct BIMObject {
  string name;
  vec<Triangle3> shells;

  BIMObject(string name, vec<Triangle3> shells);

  BIMObject();
};

typedef map<string, BIMObject> BIMObjects;

pair<BIMObjects, vec<vec<double>>> read_obj(std::ifstream &input);

enum class VoxelLabel { UNLABELED, INTERSECTED, EXTERIOR, INTERIOR };

string voxel_lable_to_string(VoxelLabel label);

typedef unsigned int RoomID;

// Maybe we only use Building and BuildingRoom
enum class CityObjectType {
  UNKOWN,
  Building,
  BuildingPart,
  BuildingRoom,
  BuildingStorey,
  BuildingUnit,
  BuildingInstallation,
  BuildingConstructiveElement,
  BuildingFurniture
};

string city_object_type_to_string(CityObjectType type);

// Maybe we use a couple of these
enum class Semantics {
  UNKOWN,
  RoofSurface,
  GroundSurface,
  WallSurface,
  ClosureSurface,
  OuterCeilingSurface,
  OuterFloorSurface,
  Window,
  Door,
  InteriorWallSurface,
  CeilingSurface,
  FloorSurface
};

struct VoxelInfo {
  VoxelLabel label;
  RoomID room_id;
  CityObjectType city_object_type;
  Semantics semantics;

  VoxelInfo();
};

// Voxel Object
struct VoxelGrid {
  vec<vec<vec<VoxelInfo>>>
      voxels; // 0: empty, 1: intersected, 2: exterior, 3 or lager: interior
  unsigned int max_x, max_y, max_z;
  // Offset is the number of additional voxels to all axis. Specifying 1 allows
  // you to have 2 addditional voxel on each minimum and maximum side of axis.
  unsigned int offset;
  vec<double> offset_origin;
  vec<double> origin;
  double resolution;

  VoxelGrid(unsigned int x, unsigned int y, unsigned int z, vec<double> origin,
            unsigned offset = 1, double resolution = 0.5);

  VoxelInfo &operator()(const unsigned int &x, const unsigned int &y,
                        const unsigned int &z);

  VoxelInfo operator()(const unsigned int &x, const unsigned int &y,
                       const unsigned int &z) const;

  vec<unsigned int> voxel_shape_with_offset() const;
};

VoxelGrid create_voxel(vec<vec<double>> vertices, unsigned int offset = 1,
                       double resolution = 0.5);

VoxelGrid intersection_with_bim_obj(const VoxelGrid &vg,
                                    const BIMObjects &bim_objs);

VoxelGrid mark_exterior_interior(const VoxelGrid &vg);

#endif