# Minimum CMake version
cmake_minimum_required(VERSION 3.16)

# Project name and language
project(output LANGUAGES CXX)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -MMD -MP")

# Define directories
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(HEADER_DIR "${CMAKE_SOURCE_DIR}/include")

# Include directories
include_directories(${HEADER_DIR})

# CPLEX Include Directories
include_directories($ENV{CPLEX_STUDIO_DIR2211}/concert/include $ENV{CPLEX_STUDIO_DIR2211}/cplex/include)

# CPLEX Library Directories
link_directories($ENV{CPLEX_STUDIO_DIR2211}/concert/lib/x86-64_linux/static_pic)
link_directories($ENV{CPLEX_STUDIO_DIR2211}/cplex/lib/x86-64_linux/static_pic)

# Linker flags
set(LINK_LIBRARIES -lconcert -lilocplex -lcplex -ldl -lpthread -lm)

# ============================
# End of CPLEX Configuration
# ============================

# Source files
file(GLOB SOURCE_FILES "${SRC_DIR}/*.cpp")

# Ensure the binary output directory exists before linking

# Add executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBRARIES})

# Include directories for the target
target_include_directories(${PROJECT_NAME} PRIVATE ${HEADER_DIR})

# Add dependency files for automatic regeneration
set_target_properties(${PROJECT_NAME} PROPERTIES 
    CMAKE_CXX_CLANG_TIDY "clang-tidy;-warnings-as-errors=*"
)

# Custom build directory for object files
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "."
)
