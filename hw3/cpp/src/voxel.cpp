#ifndef VOXEL_H
#define VOXEL_H

#include "types.h"
#include <array>
#include <cassert>
#include <limits>

VoxelGrid::VoxelGrid(unsigned int x, unsigned int y, unsigned int z,
                     vec<double> origin, unsigned offset, double resolution) {
  max_x = x;
  max_y = y;
  max_z = z;
  origin = origin;
  resolution = resolution;

  unsigned int total_voxels =
      (max_x * offset * 2) * (max_y * offset * 2) * (max_z * offset * 2);
  voxels.reserve(total_voxels);
  for (unsigned int i = 0; i < total_voxels; ++i)
    voxels.push_back(0);
}

unsigned int &VoxelGrid::operator()(const unsigned int &x,
                                    const unsigned int &y,
                                    const unsigned int &z) {
  assert(x >= 0 && x < max_x);
  assert(y >= 0 && y < max_y);
  assert(z >= 0 && z < max_z);
  return voxels[x + y * max_x + z * max_x * max_y];
}

unsigned int VoxelGrid::operator()(const unsigned int &x, const unsigned int &y,
                                   const unsigned int &z) const {
  assert(x >= 0 && x < max_x);
  assert(y >= 0 && y < max_y);
  assert(z >= 0 && z < max_z);
  return voxels[x + y * max_x + z * max_x * max_y];
}

pair<unsigned int, array<unsigned int, 3>>
world_to_voxel_index(const VoxelGrid &vg, const vec<double> &world_coord) {
  unsigned int x_num = (world_coord[0] - vg.origin[0]) / vg.resolution;
  unsigned int y_num = (world_coord[1] - vg.origin[1]) / vg.resolution;
  unsigned int z_num = (world_coord[2] - vg.origin[2]) / vg.resolution;

  int voxel_index =
      (x_num + vg.offset) * (y_num + vg.offset) * (z_num + vg.offset);
  return make_pair(voxel_index, array<unsigned int, 3>{{x_num, y_num, z_num}});
}

vec<double> world_to_voxel_center_coord(const VoxelGrid &vg,
                                        const vec<double> &world_coord) {
  array<unsigned int, 3> voxel_index =
      world_to_voxel_index(vg, world_coord).second;
  double x_coord =
      vg.origin[0] + voxel_index[0] * vg.resolution + vg.resolution / 2;
  double y_coord =
      vg.origin[1] + voxel_index[1] * vg.resolution + vg.resolution / 2;
  double z_coord =
      vg.origin[2] + voxel_index[2] * vg.resolution + vg.resolution / 2;
  return {x_coord, y_coord, z_coord};
}

// VoxelGrid voxelise(unsigned int rows_x, unsigned int rows_y,
//                    unsigned int rows_z, double resolution) {
//   return VoxelGrid(rows_x + 2, rows_y + 2, rows_z + 2);
// }

VoxelGrid create_vexel(vec<vec<double>> vertices, unsigned int offset,
                       double resolution) {
  double minx, miny, minz = numeric_limits<double>::lowest();
  double maxx, maxy, maxz = numeric_limits<double>::max();
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

  // The reason why subtracting/adding resolution is to have buffer voxel
  minx -= resolution;
  miny -= resolution;
  minz -= resolution;
  maxx += resolution;
  maxy += resolution;
  maxz += resolution;

  int num_x = static_cast<int>(ceil((maxx - minx) / resolution));
  int num_y = static_cast<int>(ceil((maxy - miny) / resolution));
  int num_z = static_cast<int>(ceil((maxz - minz) / resolution));
  vec<double> origin = {minx, miny, minz};
  return VoxelGrid(num_x, num_y, num_z, origin, offset, resolution);
}

int world_coord_to_voxel(const vec<double> &vertex, const vec<double> &origin,
                         double resolution) {
  // int vx = static_cast<int>((vertex[0] - origin[0]) / resolution);
  // int vy = static_cast<int>(vertex[1] - origin[1] / resolution);
  // int vz = static_cast<int>(vertex[2] - origin[2] / resolution);

  // vx = std::max(0, std::min(vx, static_cast<int>(grid.max_x) - 1));
  // vy = std::max(0, std::min(vy, static_cast<int>(grid.max_y) - 1));
  // vz = std::max(0, std::min(vz, static_cast<int>(grid.max_z) - 1));
}

vec<double> voxel_to_world_coord(const vec<double> &vertex,
                                 const vec<double> &origin, double resolution) {
}
vec<double> get_voxel_center(const unsigned int &x, const unsigned int &y,
                             const unsigned int &z, double resolution) {
  double center_x = x * resolution + resolution / 2;
  double center_y = y * resolution + resolution / 2;
  double center_z = z * resolution + resolution / 2;
  return vec<double>{center_x, center_y, center_z};
}

VoxelGrid intersection_with_bim_obj(const VoxelGrid &vg_arg,
                                    const BIMObjects &bim_objs_arg,
                                    double resolution) {
  const double half_res = resolution / 2;
  VoxelGrid vg(vg_arg.max_x, vg_arg.max_y, vg_arg.max_z, vg_arg.origin,
               vg_arg.offset, vg_arg.resolution);
  for (auto &voxel : vg.voxels) {
    vec<double> voxel_center =
        voxel_to_world_coord(vec<double>{0, 0, 0}, vec<double>{0, 0, 0},
                             resolution); // TODO: change later
    array<double, 6> bbox = {
        // xmin, ymin, zmin, xmax, ymax, zmax
        voxel_center[0] - half_res, voxel_center[1] - half_res,
        voxel_center[2] - half_res, voxel_center[0] + half_res,
        voxel_center[1] + half_res, voxel_center[2] + half_res};
    Bbox3 cgal_bbox =
        Bbox3(bbox[0], bbox[1], bbox[2], bbox[3], bbox[4], bbox[5]);
    bool is_intersect = false;
    for (const auto &bim : bim_objs_arg) {
      for (const auto &shell : bim.second.shells) {
        bool is_intersect = CGAL::do_intersect(
            cgal_bbox,
            shell); // TODO: do heuristic approach to reduce computation
        if (is_intersect) {
          voxel = 1; // TODO: consider to make this into group
          break;
        }
      }
      if (is_intersect) {
        break;
      }
    }
  }
  return vg;
}

#endif