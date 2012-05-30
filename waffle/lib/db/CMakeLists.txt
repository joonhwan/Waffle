project(db)

set(QT_USE_QTSQL ON)
include(${QT_USE_FILE})

message("Boost_INCLUDE_DIR = ${Boost_INCLUDE_DIR}")
message("Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
  )

set(${PROJECT_NAME}_CPP
  WSqlFirebirdDriver.cpp
  WSqlFirebirdTableBuilder.cpp
  WSqlFirebirdTableCreateQueryBuilder.cpp
  WSqlMultiQuery.cpp
  WSqlQueryBinder.cpp
  WSqlQueryBuilder.cpp
  WSqlTableFieldNameList.cpp
  WSqlTableLister.cpp
  )

set(${PROJECT_NAME}_H
  WSqlFirebirdTableBuilder.h
  WSqlFirebirdTableCreateQueryBuilder.h
  WSqlHelper.h
  WSqlMultiQuery.h
  WSqlQueryBinder.h
  WSqlQueryBuilder.h
  WSqlTableFieldNameList.h
  WSqlTableLister.h
  )

set(${PROJECT_NAME}_MOC_H
  WSqlFirebirdDriver.h
  )

set(${PROJECT_NAME}_QRC
  )

qt4_wrap_cpp(${PROJECT_NAME}_MOC_CPP ${${PROJECT_NAME}_MOC_H})
qt4_add_resources(${PROJECT_NAME}_QRC_CPP ${${PROJECT_NAME}_QRC})

# add_definitions(
#   -D_CRT_SECURE_NO_WARNINGS
#   )

add_library(${PROJECT_NAME}
  ${${PROJECT_NAME}_CPP}
  ${${PROJECT_NAME}_MOC_CPP}
  ${${PROJECT_NAME}_MOC_H}
  ${${PROJECT_NAME}_CPP_WITH_MOC}
  ${${PROJECT_NAME}_MOC_FROM_CPP}
  ${${PROJECT_NAME}_QRC_CPP}
  ${${PROJECT_NAME}_MOC_CPP_INCLUDED_BY_SOURCE}
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  FOLDER "waffle"
  )

target_link_libraries(${PROJECT_NAME}
  firebird	   # external/firebird lib
  flamerobin
  common
  advapi32.lib # for win32 service api
  )
