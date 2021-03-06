cmake_minimum_required (VERSION 2.8)
project(cbProject)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${CMAKE_CURRENT_LIST_DIR}/ThirdParty/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/cbLib)
include_directories(${CMAKE_CURRENT_LIST_DIR})

link_directories(${CMAKE_CURRENT_LIST_DIR}/ThirdParty/lib)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(cbLib)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/ThirdParty/bin/glew32.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY ${CMAKE_CURRENT_LIST_DIR}/ThirdParty/bin/assimp-vc140-mt.dll DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY ${CMAKE_CURRENT_LIST_DIR}/res DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

file(GLOB SOURCE
    "*.h"
    "*.cpp"
)

set(SHADERS shaders/imgui.f
			shaders/imgui.v
			shaders/noise.f
			shaders/noise.v
			shaders/model.f
			shaders/model.v
			shaders/pbr.f
			shaders/pbr.v)

add_library(cbGame SHARED ${SOURCE} ${SHADERS})
source_group("shaders" FILES ${SHADERS})
target_link_libraries(cbGame glew32 opengl32 assimp-vc140-mt)