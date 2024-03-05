cmake_minimum_required(VERSION 3.27)
project(hw1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CGAL REQUIRED)
include(${CGAL_USE_FILE})
file(GLOB SOURCES *.h *.cpp)

add_executable(hw1 hw1.cpp)

target_link_libraries(hw1 CGAL::CGAL)