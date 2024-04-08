#ifndef VOXEL_GRID_H
#define VOXEL_GRID_H

#include "types.h"
#include "voxelinfo.cpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>

VoxelGrid::VoxelGrid(unsigned int x, unsigned int y, unsigned int z,
                     vec<double> origin, unsigned offset, double resolution) {
  this->max_x = x;
  this->max_y = y;
  this->max_z = z;
  this->origin = origin;
  this->resolution = resolution;
  this->offset = offset;
  this->offset_origin = {origin[0] - offset * resolution,
                         origin[1] - offset * resolution,
                         origin[2] - offset * resolution};

  unsigned int total_voxels =
      (max_x + offset * 2) * (max_y + offset * 2) * (max_z + offset * 2);
  voxels.reserve(total_voxels);
  for (unsigned int i = 0; i < max_x + offset * 2; ++i) {
    vec<vec<VoxelInfo>> y_voxels;
    for (unsigned int j = 0; j < max_y + offset * 2; ++j) {
      vec<VoxelInfo> z_voxels;
      for (unsigned int k = 0; k < max_z + offset * 2; ++k) {
        VoxelInfo voxel_info = VoxelInfo();
        voxel_info.label = VoxelLabel::UNLABELED;
        z_voxels.push_back(voxel_info);
      }
      y_voxels.push_back(z_voxels);
    }
    voxels.push_back(y_voxels);
  }
}

VoxelInfo &VoxelGrid::operator()(const unsigned int &x, const unsigned int &y,
                                 const unsigned int &z) {
  assert(x >= 0 && x < max_x);
  assert(y >= 0 && y < max_y);
  assert(z >= 0 && z < max_z);
  return voxels[x][y][z];
}

VoxelInfo VoxelGrid::operator()(const unsigned int &x, const unsigned int &y,
                                const unsigned int &z) const {
  assert(x >= 0 && x < max_x);
  assert(y >= 0 && y < max_y);
  assert(z >= 0 && z < max_z);
  return voxels[x][y][z];
}

vec<unsigned int> VoxelGrid::voxel_shape_with_offset() const {
  return {max_x + offset * 2, max_y + offset * 2, max_z + offset * 2};
};

vec<double> voxel_index_to_coordinate(const VoxelGrid &vg,
                                      const vec<unsigned int> &voxel_xyz) {
  double x_coord =
      vg.offset_origin[0] + voxel_xyz[0] * vg.resolution + vg.resolution / 2;
  double y_coord =
      vg.offset_origin[1] + voxel_xyz[1] * vg.resolution + vg.resolution / 2;
  double z_coord =
      vg.offset_origin[2] + voxel_xyz[2] * vg.resolution + vg.resolution / 2;
  return {x_coord, y_coord, z_coord};
  ;
}

VoxelGrid create_voxel(vec<vec<double>> vertices, unsigned int offset,
                       double resolution) {
  double minx, miny, minz = numeric_limits<double>::max();
  double maxx, maxy, maxz = numeric_limits<double>::min();

  for (const auto &vertex : vertices) {
    if (vertex.size() >= 3) {
      minx = min(minx, vertex[0]);
      miny = min(miny, vertex[1]);
      minz = min(minz, vertex[2]);

      maxx = max(maxx, vertex[0]);
      maxy = max(maxy, vertex[1]);
      maxz = max(maxz, vertex[2]);
    }
  }
  double x_diff = maxx - minx;
  cout << "x diff :" << x_diff << endl;
  int num_x = static_cast<int>(ceil((maxx - minx) / resolution));
  int num_y = static_cast<int>(ceil((maxy - miny) / resolution));
  int num_z = static_cast<int>(ceil((maxz - minz) / resolution));

  cout << "size of voxel grid 1: " << num_x << " " << num_y << " " << num_z
       << endl;
  vec<double> origin = {minx, miny, minz};
  return VoxelGrid(num_x, num_y, num_z, origin, offset, resolution);
}

bool simple_intersection(
    const Triangle3 &shell,
    const vec<double>
        &voxel_bbox) { // voxel_bbox is xmin, ymin, zmin, xmax, ymax, zmax
  // if bbox of shell and voxel_bbox overlaps return true otherwise false

  // xmin, ymin, zmin, xmax, ymax, zmax
  const Point3 p0 = shell.vertex(0);
  const Point3 p1 = shell.vertex(1);
  const Point3 p2 = shell.vertex(2);

  const double xmin = min({p0.x(), p1.x(), p2.x()});
  const double ymin = min({p0.y(), p1.y(), p2.y()});
  const double zmin = min({p0.z(), p1.z(), p2.z()});
  const double xmax = max({p0.x(), p1.x(), p2.x()});
  const double ymax = max({p0.y(), p1.y(), p2.y()});
  const double zmax = max({p0.z(), p1.z(), p2.z()});

  bool overlap_x = (xmax >= voxel_bbox[0]) && (xmin <= voxel_bbox[3]);
  bool overlap_y = (ymax >= voxel_bbox[1]) && (ymin <= voxel_bbox[4]);
  bool overlap_z = (zmax >= voxel_bbox[2]) && (zmin <= voxel_bbox[5]);

  return overlap_x && overlap_y && overlap_z;
}

VoxelGrid intersection_with_bim_obj(const VoxelGrid &vg_arg,
                                    const BIMObjects &bim_objs_arg) {
  const double resolution = vg_arg.resolution;
  const double half_res = resolution / 2;
  VoxelGrid vg(vg_arg.max_x, vg_arg.max_y, vg_arg.max_z, vg_arg.origin,
               vg_arg.offset, vg_arg.resolution);
  for (unsigned int x = 0; x < vg.voxels.size(); x++) {
    for (unsigned int y = 0; y < vg.voxels[x].size(); y++) {
      for (unsigned int z = 0; z < vg.voxels[x][y].size(); z++) {
        vec<unsigned int> voxel_xyz = {x, y, z};
        vec<double> voxel_center = voxel_index_to_coordinate(vg, voxel_xyz);

        // xmin, ymin, zmin, xmax, ymax, zmax
        vec<double> bbox = {
            voxel_center[0] - half_res, voxel_center[1] - half_res,
            voxel_center[2] - half_res, voxel_center[0] + half_res,
            voxel_center[1] + half_res, voxel_center[2] + half_res};
        Bbox3 cgal_bbox =
            Bbox3(bbox[0], bbox[1], bbox[2], bbox[3], bbox[4], bbox[5]);
        for (const auto &bim : bim_objs_arg) {
          for (const auto &shell : bim.second.shells) {
            bool is_simply_intersect = simple_intersection(shell, bbox);
            if (!is_simply_intersect) {
              continue;
            }
            bool is_intersect = CGAL::do_intersect(cgal_bbox, shell);
            if (is_intersect) {
              vg.voxels[x][y][z].label = VoxelLabel::INTERSECTED;
              if (bim.second.sem == GeometricSemantics::Other) {
                vg.voxels[x][y][z].semantics = Semantics::UNKOWN;
              } else if (bim.second.sem == GeometricSemantics::Roof) {
                vg.voxels[x][y][z].semantics = Semantics::RoofSurface;
              } else if (bim.second.sem == GeometricSemantics::Floor) {
                vg.voxels[x][y][z].semantics = Semantics::FloorSurface;
              } else if (bim.second.sem == GeometricSemantics::Wall) {
                vg.voxels[x][y][z].semantics = Semantics::WallSurface;
              } else if (bim.second.sem == GeometricSemantics::Window) {
                vg.voxels[x][y][z].semantics = Semantics::Window;
              } else if (bim.second.sem == GeometricSemantics::Door) {
                vg.voxels[x][y][z].semantics = Semantics::Door;
              } else if (bim.second.sem == GeometricSemantics::InteriorWall) {
                vg.voxels[x][y][z].semantics = Semantics::InteriorWallSurface;
              } else {
                vg.voxels[x][y][z].semantics = Semantics::UNKOWN;
              }
              break;
            }
          }
        }
      }
    }
  }

  return vg;
}

const vec<vec<int>> six_connectivity = {{-1, 0, 0}, {1, 0, 0},  {0, -1, 0},
                                        {0, 1, 0},  {0, 0, -1}, {0, 0, 1}};

const vec<vec<int>> eighteen_connectivity = { // 6-connectivity
    {-1, 0, 0},
    {1, 0, 0},
    {0, -1, 0},
    {0, 1, 0},
    {0, 0, -1},
    {0, 0, 1},
    // rest of 12-connectivity
    {-1, -1, 0},
    {1, -1, 0},
    {-1, 1, 0},
    {1, 1, 0},
    {-1, 0, -1},
    {1, 0, -1},
    {-1, 0, 1},
    {1, 0, 1},
    {0, -1, -1},
    {0, 1, -1},
    {0, -1, 1},
    {0, 1, 1}};
const vec<vec<int>> twenty_six_connectivity = { // 6-connectivity
    {-1, 0, 0},
    {1, 0, 0},
    {0, -1, 0},
    {0, 1, 0},
    {0, 0, -1},
    {0, 0, 1},
    // rest of 12-connectivity
    {-1, -1, 0},
    {1, -1, 0},
    {-1, 1, 0},
    {1, 1, 0},
    {-1, 0, -1},
    {1, 0, -1},
    {-1, 0, 1},
    {1, 0, 1},
    {0, -1, -1},
    {0, 1, -1},
    {0, -1, 1},
    {0, 1, 1},
    // rest of 8-connectivity
    {-1, -1, -1},
    {1, -1, -1},
    {-1, 1, -1},
    {1, 1, -1},
    {-1, -1, 1},
    {1, -1, 1},
    {-1, 1, 1},
    {1, 1, 1}};

void mark_interior(VoxelGrid &vg_marked, int x, int y, int z,
                   unsigned int interior_id, const vec<vec<int>> &connectivity,
                   const vec<unsigned int> &voxel_shape) {
  if (x < 0 || y < 0 || z < 0 || x >= voxel_shape[0] || y >= voxel_shape[1] ||
      z >= voxel_shape[2]) {
    return;
  }

  // Only mark if voxel is currently unmarked (interior and not visited)
  if (vg_marked.voxels[x][y][z].label == VoxelLabel::UNLABELED) {
    vg_marked.voxels[x][y][z].label = VoxelLabel::INTERIOR;
    vg_marked.voxels[x][y][z].room_id = interior_id;

    for (const auto &adjacent_voxel : connectivity) {
      int adj_x = x + adjacent_voxel[0];
      int adj_y = y + adjacent_voxel[1];
      int adj_z = z + adjacent_voxel[2];
      if (adj_x >= voxel_shape[0] || adj_y >= voxel_shape[1] ||
          adj_z >= voxel_shape[2] || adj_x < 0 || adj_y < 0 || adj_z < 0) {
        continue;
      }
      mark_interior(vg_marked, adj_x, adj_y, adj_z, interior_id, connectivity,
                    voxel_shape);
    }
  }
}

VoxelGrid mark_exterior_interior(VoxelGrid &vg) {
  cout << "===Marking exterior and interior voxels===\n";
  VoxelGrid vg_marked(vg.max_x, vg.max_y, vg.max_z, vg.origin, vg.offset,
                      vg.resolution);

  // Mark exteriror first
  vec<unsigned int> voxel_shape = vg.voxel_shape_with_offset();
  vg_marked.voxels = vg.voxels;
  for (unsigned int x = 0; x < vg.voxels.size(); x++) {
    for (unsigned int y = 0; y < vg.voxels[x].size(); y++) {
      for (unsigned int z = 0; z < vg.voxels[x][y].size(); z++) {
        // TODO: consider conectivity
        // list up adjacent voxels
        if (x == 0 && y == 0 && z == 0) { // Start marking from the first voxel
          vg_marked.voxels[x][y][z].label = VoxelLabel::EXTERIOR; // exterior
        }

        // If the voxel is exterior, mark all adjacent voxels as exterior
        if (vg_marked.voxels[x][y][z].label == VoxelLabel::EXTERIOR) {
          for (auto const &adjacent_voxel : eighteen_connectivity) {
            int adj_x = x + adjacent_voxel[0];
            int adj_y = y + adjacent_voxel[1];
            int adj_z = z + adjacent_voxel[2];

            // Skip if adjacent voxel is out of bounds
            if (adj_x >= voxel_shape[0] || adj_y >= voxel_shape[1] ||
                adj_z >= voxel_shape[2] || adj_x < 0 || adj_y < 0 ||
                adj_z < 0) {
              continue;
            }
            // Only when it's empty, mark as exterior so that it doesn't
            // overwrite marked voxels;
            if (vg_marked.voxels[adj_x][adj_y][adj_z].label ==
                VoxelLabel::UNLABELED) {
              vg_marked.voxels[adj_x][adj_y][adj_z].label =
                  VoxelLabel::EXTERIOR; // exterior
            }
          }
        }
      }
    }
  }

  unsigned int interior_id = 0; //
  for (unsigned int x = 0; x < vg.voxels.size(); x++) {
    for (unsigned int y = 0; y < vg.voxels[x].size(); y++) {
      for (unsigned int z = 0; z < vg.voxels[x][y].size(); z++) {
        // If the voxel is not marked as exterior, mark as interior
        if (vg_marked.voxels[x][y][z].label == VoxelLabel::UNLABELED) {
          mark_interior(vg_marked, x, y, z, interior_id, eighteen_connectivity,
                        voxel_shape);
          interior_id++;
        }
      }
    }
  }

  cout << "num of classes: " << interior_id << "\n";

  return vg_marked;
}

void extract_surface(VoxelGrid &vg,
                     vec<vec<int>> connectivity = eighteen_connectivity) {
  for (unsigned int x = 0; x < vg.voxels.size(); x++) {
    for (unsigned int y = 0; y < vg.voxels[x].size(); y++) {
      for (unsigned int z = 0; z < vg.voxels[x][y].size(); z++) {
        vec<pair<int, VoxelLabel>> neighbour_labels;
        for (int i = 0; i < connectivity.size(); i++) {
          auto adjacent_voxel = connectivity[i];
          int adj_x = x + adjacent_voxel[0];
          int adj_y = y + adjacent_voxel[1];
          int adj_z = z + adjacent_voxel[2];
          if (adj_x >= vg.voxels.size() || adj_y >= vg.voxels[x].size() ||
              adj_z >= vg.voxels[x][y].size() || adj_x < 0 || adj_y < 0 ||
              adj_z < 0) {
            continue;
          }
          neighbour_labels.push_back(
              make_pair(i, vg.voxels[adj_x][adj_y][adj_z].label));
        };

        if (vg.voxels[x][y][z].label == VoxelLabel::INTERSECTED) {
          if (any_of(neighbour_labels.begin(), neighbour_labels.end(),
                     [](const auto &pair) {
                       return pair.second == VoxelLabel::EXTERIOR;
                     })) {
            vg.voxels[x][y][z].city_object_type = CityObjectType::BuildingPart;
            continue;
          }
          auto found = find_if(neighbour_labels.begin(), neighbour_labels.end(),
                               [](const auto &pair) {
                                 return pair.second == VoxelLabel::INTERIOR;
                               });
          if (found != neighbour_labels.end()) {
            auto neighbour_xyz = connectivity[found->first];
            auto neighbour =
                vg.voxels[x + neighbour_xyz[0]][y + neighbour_xyz[1]]
                         [z + neighbour_xyz[2]];
            vg.voxels[x][y][z].city_object_type = CityObjectType::BuildingRoom;
            vg.voxels[x][y][z].room_id = neighbour.room_id;
          } else {
            vg.voxels[x][y][z].city_object_type = CityObjectType::BuildingRoom;
          }
        }
      }
    }
  }
}

#endif