cmake_minimum_required(VERSION 3.14)
project(hw2)

# set(CMAKE_CXX_COMPILER g++)
# set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
# set(CMAKE_BUILD_TYPE "Release")
# set(CMAKE_CXX_FLAGS "-O2" )
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(${PROJECT_NAME}_VERSION_MAJOR 1)
set(${PROJECT_NAME}_VERSION_MINOR 0)

include_directories(${CMAKE_SOURCE_DIR}/include/)

# CGAL
find_package(CGAL QUIET COMPONENTS)
if (CGAL_FOUND)
    include(${CGAL_USE_FILE})
    message(STATUS ${CGAL_LIBRARIES})
    message(STATUS ${CGAL_3RD_PARTY_LIBRARIES})
else ()
    message(SEND_ERROR "CGAL library is required")
    return()
endif ()

find_package(Eigen3 3.4.0 QUIET)
include(CGAL_Eigen3_support)

FILE(GLOB SRC_FILES src/*.cpp)
add_executable(hw2 ${SRC_FILES}
        src/main.cpp)


target_link_libraries(${PROJECT_NAME} CGAL::CGAL CGAL::Eigen3_support)
