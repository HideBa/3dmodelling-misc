## 1. Steps to Run the Code

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
        ./hw2
        ```

This structured approach ensures clarity and facilitates a smooth setup process for running the program.
