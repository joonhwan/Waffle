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
# essential ?
find_package(Qt4 REQUIRED)
find_package(Boost REQUIRED)
find_package(OpenCV REQUIRED)
# non-essential ?
find_package(Vld) # visual leak detector library
find_package(Mil) # matrox imaging library
find_package(Ipp) # intel ipp
# for easy spell...
if(Vld_FOUND)
  set(VLD_FOUND)
endif()
if(Mil_FOUND)
  set(MIL_FOUND)
endif()
if(Ipp_FOUND)
  set(IPP_FOUND ON)
endif()

#  Path for build/platform -specific headers
set(WAFFLE_CONFIG_FILE_INCLUDE_DIR
  "${CMAKE_BINARY_DIR}/" CACHE PATH "Where to create the platform-dependant cvconfig.h"
  )
add_definitions(-DHAVE_WAFFLE_CONFIG_H)
include_directories(${WAFFLE_CONFIG_FILE_INCLUDE_DIR})

configure_file(
  ${Waffle_SOURCE_DIR}/cmake/WaffleConfig.h.cmake
  ${WAFFLE_CONFIG_FILE_INCLUDE_DIR}/WaffleConfig.h
  )

# everything needs Qt
include(${QT_USE_FILE})

add_subdirectory(waffle)
add_subdirectory(example)
add_subdirectory(test)