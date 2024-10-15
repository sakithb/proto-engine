cmake_minimum_required(VERSION 3.30)
project(proto-engine)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Libraries

add_subdirectory(vendor/glfw)

add_subdirectory(vendor/glad/cmake)
glad_add_library(glad REPRODUCIBLE API gl:core=4.6)

set(CGLM_STATIC ON)
add_subdirectory(vendor/cglm EXCLUDE_FROM_ALL)

add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE vendor/cgltf)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE vendor/stb)

# Targets

set(BUILD_SHARED_LIBS OFF)
include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE ENGINE_SRCS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.c) 
add_executable(engine ${ENGINE_SRCS})
set_property(TARGET engine PROPERTY C_STANDARD 11)
target_link_libraries(engine PRIVATE glad glfw cgltf stb cglm_headers m)
