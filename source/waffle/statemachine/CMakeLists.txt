PROJECT(statemachine)

INCLUDE(${QT_USE_FILE})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SET(sources
  wactive.cpp
  wevent.cpp
  whsm.cpp
  wtimer.cpp
  )

SET(moc_header
  wactive.h
  whsm.h
  wtimer.h
  )

set(header
  wevent.h
  ${moc_header}
  )

SET(forms
  )

SET(resources
  )

QT4_WRAP_CPP(moc_generated ${moc_header})
QT4_WRAP_UI(forms_generated ${forms})
QT4_ADD_RESOURCES(rc_generated ${resources})

ADD_LIBRARY(${PROJECT_NAME} STATIC
  ${sources}
  ${header}
  ${moc_header}
  ${form}
  ${resource}
  ${moc_generated}
  ${forms_generated}
  ${rc_generated}
  )

set_source_files_properties(${moc_generated}
  PROPERTIES HEADER_FILE_ONLY
  true)

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  FOLDER "waffle"
  )