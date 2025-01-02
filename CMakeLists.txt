# Minimum CMake version
cmake_minimum_required(VERSION 3.16)

# Project name and language
project(output LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -MMD -MP")

# Define directories
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(HEADER_DIR "${CMAKE_SOURCE_DIR}/include")

# Include directories
include_directories(${HEADER_DIR})

set(cplex_dir "$ENV{CPLEX_STUDIO_DIR2211}")

# CPLEX Include Directories
include_directories(${cplex_dir}/concert/include ${cplex_dir}/cplex/include)

# CPLEX Library Directories
link_directories(${cplex_dir}/concert/lib/x86-64_linux/static_pic)
link_directories(${cplex_dir}/cplex/lib/x86-64_linux/static_pic)

# Linker flags
set(LINK_LIBRARIES -lconcert -lilocplex -lcplex -ldl -lpthread -lm)

# Source files
file(GLOB SOURCE_FILES "${SRC_DIR}/unit_tests.cpp")
file(GLOB SOURCE_FILES "${SRC_DIR}/*.cpp")

# Add executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC ${LINK_LIBRARIES})

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
