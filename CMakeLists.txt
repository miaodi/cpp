cmake_minimum_required(VERSION 3.10)
project(untitled3)

set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_C_COMPILER mpicc)
# set(CMAKE_CXX_COMPILER mpicxx)

# list (APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake-modules")

# find_package (PETSc REQUIRED)
# message("here is PETSc: ${PETSC_DIR}")

# include_directories (${PETSC_INCLUDES} )
# add_definitions (${PETSC_DEFINITIONS} )

FIND_PACKAGE( Boost COMPONENTS program_options REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

include_directories ("${PROJECT_SOURCE_DIR}/tinyexpr")
add_subdirectory (tinyexpr) 

add_executable(exe main.cpp)
TARGET_LINK_LIBRARIES( exe LINK_PUBLIC ${Boost_LIBRARIES} Tinyexpr )