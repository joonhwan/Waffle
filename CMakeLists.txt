cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

enable_testing()

project(Waffle)

set(PROJECT_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${PROJECT_CMAKE_DIR})

set_property(GLOBAL PROPERTY USE_FOLDERS "ON")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/waffle/external)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/waffle/lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/waffle/lib+)

if(MSVC)
  set(BUILD_64 ${CMAKE_CL_64})
else(MSVC)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(BUILD_64 ON)
  else(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(BUILD_64 OFF)
  endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
endif(MSVC)

# Set up the environment
find_package(Qt4 REQUIRED)
find_package(Vld REQUIRED)
find_package(Ipp REQUIRED)
find_package(Boost REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Mil QUIET)
find_package(GraphViz REQUIRED)

# everything needs Qt
include(${QT_USE_FILE})

add_subdirectory(waffle)
add_subdirectory(example)
add_subdirectory(test)