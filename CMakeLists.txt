cmake_minimum_required(VERSION 3.30)
project(proto-engine)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Libraries

add_subdirectory(vendor/glfw)

add_subdirectory(vendor/glad/cmake)
glad_add_library(glad REPRODUCIBLE API gl:core=4.6)

add_subdirectory(vendor/cglm)

set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(vendor/assimp)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE vendor/stb)

# Targets

set(BUILD_SHARED_LIBS OFF)
include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE ENGINE_SRCS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/engine/*.c) 
add_executable(engine ${ENGINE_SRCS})
target_link_libraries(engine glad glfw cglm)

file(GLOB_RECURSE MESH_LOADER_SRCS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/mesh_loader/*.c) 
add_executable(mesh_loader ${MESH_LOADER_SRCS})
target_link_libraries(mesh_loader assimp stb)
