cmake_minimum_required(VERSION 3.14)
project(hw3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64")

include_directories(${CMAKE_SOURCE_DIR}/include/)

enable_testing()

# CGAL
find_package(CGAL QUIET COMPONENTS Core)
if (CGAL_FOUND)
    include(${CGAL_USE_FILE})
    message(STATUS "CGAL Libraries: ${CGAL_LIBRARIES}")
    message(STATUS "CGAL 3rd Party Libraries: ${CGAL_3RD_PARTY_LIBRARIES}")
else ()
    message(SEND_ERROR "CGAL library is required but not found.")
    return()
endif ()

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/release-1.10.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # Use shared (DLL) run-time lib even when Google Test is built as static lib.
FetchContent_MakeAvailable(googletest)

# Library Target
file(GLOB SRC_LIB_FILES src/*.cpp)
# Exclude main.cpp from the library sources
list(REMOVE_ITEM SRC_LIB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
add_library(${PROJECT_NAME}_lib ${SRC_LIB_FILES})

# Profiling
option(ENABLE_PROFILING "Enable profiling" OFF)
if (ENABLE_PROFILING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    message(STATUS "Profiling enabled")
endif ()


# Main Executable Target
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_lib ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES})

# Test Executable Target
add_executable(${PROJECT_NAME}_test src/tests/test_io.cpp)
target_link_libraries(${PROJECT_NAME}_test gtest_main ${PROJECT_NAME}_lib)
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test)
