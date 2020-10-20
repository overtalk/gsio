cmake_minimum_required(VERSION 3.0)

#--------------------------
# Mesh project
#--------------------------
project(gsio)

set(CMAKE_CXX_STANDARD 11)
set(ROOT_DIR ${PROJECT_SOURCE_DIR})

include_directories(
	${ROOT_DIR}/3rd
	${ROOT_DIR}/include
)

set(BIN_OUTPUT_DIR ${ROOT_DIR}/bin/bin)
set(DEP_LIB_DIR ${BIN_OUTPUT_DIR})

#--------------------------
# Building mesh source.
#--------------------------
message(STATUS "Start to build all source...")
add_subdirectory(examples)