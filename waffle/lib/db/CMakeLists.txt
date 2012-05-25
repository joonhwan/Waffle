project(db)

set(QT_USE_QTSQL ON)
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(${PROJECT_NAME}_CPP
  WSqlFirebirdDriver.cpp
  WSqlQueryBinder.cpp
  WSqlQueryBuilder.cpp
  WSqlFirebirdTableCreateQueryBuilder.cpp
  WSqlTableFieldNameList.cpp
  WSqlTableLister.cpp
  )

set(${PROJECT_NAME}_H
  WSqlHelper.h
  WSqlQueryBinder.h
  WSqlQueryBuilder.h
  WSqlFirebirdTableCreateQueryBuilder.h
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
  common
  advapi32.lib # for win32 service api
  )
