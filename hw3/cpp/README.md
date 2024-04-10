# 1. Steps to Run the Code

To successfully run the code, follow these preparatory steps to ensure all configurations and modifications align with
your system requirements:

1. **CMake Version Check**: The code requires CMake version 3.27 or higher. Verify your CMake installation meets this
   requirement to avoid compatibility issues.
2. **File Path Configuration**: Locate the line in the code specifying the path to the OBJ file and the output directory
   for the OBJ file. Update these paths to reflect the correct locations on your system, ensuring the program can access
   necessary files.
3. **Execution Instructions**: Follow the commands below to build and run the program. Open your terminal or command
   prompt and execute these steps in order:

    - **Prepare the Build Directory**:

        ```bash
        mkdir build && cd build
        ```

    - **Configure the Project with CMake**:

        ```bash
        cmake ..
        ```

    - **Build the Program**:

        ```bash
        make
        ```

    - **Run the Program**:

        ```bash
        ./hw3
        ```

    - **If you want to run test code**:
      Uncomment where commented out in `CMakeLists.txt`
      then, run

        ```bash
        ./hw3_test
        ```

This structured approach ensures clarity and facilitates a smooth setup process for running the program.

## Files

### Programs

./src/
├── bim_obj.cpp: has operation of struct read from OBJ file
├── cjson.cpp: exports voxel as CityJSON format
├── io.cpp: reads and writes general files
├── main.cpp: Entry point
├── tests
│ ├── test_io.cpp
│ └── testdata
│ └── open_house_ifc4.obj
├── types.h: defines struct and other common types for other code bases.
├── voxelgrid.cpp: defines voxel grid and its corresponding methods
└── voxelinfo.cpp: defines single voxel

### Data files

./hw3_submission/data
├── input
│ ├── open_house4.ifc: Original IFC file
│ ├── open_house_ifc4.obj: OBJ file converted by IfcConvert
│ ├── wellness_center_sama.obj: OBJ file converted by IfcConvert
│ └── wellness_center_sama_trimmed.ifc: Original IFC file
└── output
│ ├── out_open_house.city.json: CityJSON file which has voxelised BIM model. (resolution: 0.3)
│ ├── voxel_open_house.obj: OBJ file after voxelised. This is same as what exported as CityJSON. (Mainly for debug and
visual interpretation purpose)
│ ├── voxel_wellness_sama.obj: OBJ file after voxelised. This is same as what exported as CityJSON. (Mainly for debug
and visual interpretation purpose)
│ └── wellness_center_sama.city.json: CityJSON file which has voxelised BIM model. (resolution: 0.3)
