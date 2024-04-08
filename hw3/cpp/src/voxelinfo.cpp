#ifndef VOXEL_INFO_H
#define VOXEL_INFO_H

#include "types.h"
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>

VoxelInfo::VoxelInfo() {
  this->label = VoxelLabel::UNLABELED;
  this->city_object_type = CityObjectType::UNKOWN;
  this->semantics = Semantics::UNKOWN;
  this->room_id = 0;
}

string voxel_lable_to_string(VoxelLabel label) {
  switch (label) {
  case VoxelLabel::UNLABELED:
    return "UNLABELED";
  case VoxelLabel::INTERSECTED:
    return "INTERSECTED";
  case VoxelLabel::EXTERIOR:
    return "EXTERIOR";
  case VoxelLabel::INTERIOR:
    return "INTERIOR";
  default:
    return "UNKNOWN";
  }
}

#endif