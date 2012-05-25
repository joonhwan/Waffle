# find Matrox Imaging Library depending build environment

if(MSVC)
  set(Find_Mil_64 ${CMAKE_CL_64})
else()
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(Find_Mil_64 ON)
  else()
	set(Find_Mil_64 OFF)
  endif()
endif()

if(Find_Mil_64)
  set(Mil_INSTALL_DIRS
	"${Mil_DIR}"
	"$ENV{ProgramW6432}/Matrox Imaging/Mil"
	"$ENV{ProgramFiles}/Matrox Imaging/Mil"
	"c:/dev/sdk/mil64/mil"
	)
else()
  set(Mil_INSTALL_DIRS
	"${Mil_DIR}"
	"$ENV{ProgramFiles}/Matrox Imaging/Mil"
	"$ENV{ProgramFiles(x86)}/Matrox Imaging/Mil"
	"c:/dev/sdk/mil32/mil"
	)
endif()

find_path(Mil_INCLUDE_DIR
  NAMES Mil.h
  HINTS ${Mil_INSTALL_DIRS}
  PATH_SUFFIXES "include"
  )

find_path(Mil_LIB_DIR
  NAMES mil.lib 
  HINTS ${Mil_INSTALL_DIRS}
  PATH_SUFFIXES "lib"
  )

set(Mil_LIBRARIES "general;${MIL_LIB_DIR}/mil.lib")
set(Mil_INCLUDE_DIRS ${Mil_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set Mil_FOUND to TRUE if 
# all listed variables are TRUE
include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
find_package_handle_standard_args(Mil DEFAULT_MSG Mil_LIBRARY Mil_INCLUDE_DIR)
# message("vld include dirs=${Mil_INCLUDE_DIRS}")
