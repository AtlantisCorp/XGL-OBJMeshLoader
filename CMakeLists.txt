# !! File OBJMeshLoader/CMakeLists.txt
# !! Author Luk2010
# !! Date 2021/07/08

cmake_minimum_required(VERSION 3.20)
project(OBJMeshLoader)

file(GLOB OBJMeshLoaderSrc ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(OBJMeshLoader SHARED ${OBJMeshLoaderSrc})

install(
    TARGETS OBJMeshLoader 
    LIBRARY DESTINATION ${XGL_PLUGINS_BINDIR}
)