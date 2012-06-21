project(WaffleLibPlusFa)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(QT_USE_QAXCONTAINER ON)
include(${QT_USE_FILE})

set(xcom_sources
  xcom/excomlib.cpp
  xcom/excomvblib.cpp
  xcom/XCom.cpp
  xcom/XComVb.cpp
  )
set(xcom_header
  xcom/excomlib.h
  xcom/excomvblib.h
  xcom/XCom.h
  xcom/XComVb.h
  )

set(sources
  PlusDummy.cpp
  )

set(headers
  )

if(WIN32 AND NOT BUILD_64)
  set(sources ${sources} ${xcom_sources})
  set(headers ${headers} ${xcom_headers})
endif()

set(moc_header
  )

set(forms
  )

set(resources
  )

qt4_wrap_cpp(moc_generated ${moc_header})
qt4_wrap_ui(forms_generated ${forms})
qt4_add_resources(rc_generated ${resources})

add_library(${PROJECT_NAME}
  ${sources}
  ${header}
  ${moc_header}
  ${form}
  ${resource}
  ${moc_generated}
  ${forms_generated}
  ${rc_generated}
  )

target_link_libraries(${PROJECT_NAME}
  ${QT_LIBRARIES}
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  FOLDER "waffle+"
  )