project(firebird)

set(QT_USE_QTSQL ON)
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(${PROJECT_NAME}_CPP
  qsql_ibase.cpp
  )

set(${PROJECT_NAME}_H
  ibase.h
  iberror.h
  ib_util.h
  )

set(${PROJECT_NAME}_MOC_H
  qsql_ibase.h
  )

set(${PROJECT_NAME}_QRC
  )

qt4_wrap_cpp(${PROJECT_NAME}_MOC_CPP ${${PROJECT_NAME}_MOC_H})
qt4_add_resources(${PROJECT_NAME}_QRC_CPP ${${PROJECT_NAME}_QRC})

add_definitions(
  -D_CRT_SECURE_NO_WARNINGS
  )

add_library(${PROJECT_NAME}
  ${${PROJECT_NAME}_CPP}
  ${${PROJECT_NAME}_MOC_CPP}
  ${${PROJECT_NAME}_MOC_H}
  ${${PROJECT_NAME}_CPP_WITH_MOC}
  ${${PROJECT_NAME}_MOC_FROM_CPP}
  ${${PROJECT_NAME}_QRC_CPP}
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  FOLDER "externals"
  )

if(BUILD_64)
  set(FBCLIENT_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/x64/fbclient_ms.lib)
else(BUILD_64)
  set(FBCLIENT_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/x86/fbclient_ms.lib)
endif(BUILD_64)

target_link_libraries(${PROJECT_NAME}
  ${FBCLIENT_LIB}
  # ${CMAKE_CURRENT_SOURCE_DIR}/ib_util_ms.lib
  advapi32.lib # for win32 service api
  )
