PROJECT(statemachine_test)

SET(QT_USE_QTMAIN ON)
INCLUDE(${QT_USE_FILE})

SET(sources
  myevents.cpp
  testerwindow.cpp
  main.cpp
  mystatemachine.cpp
  )

SET(moc_header
  myeventids.h
  myevents.h
  mystatemachine.h
  testerwindow.h
  )

SET(header
  ${moc_header}
  )

SET(forms
  )

SET(resources
  )

QT4_WRAP_CPP(moc_generated ${moc_header})
QT4_WRAP_UI(forms_generated ${forms})
QT4_ADD_RESOURCES(rc_generated ${resources})

if(CMAKE_HOST_WIN32)
set(CMAKE_BUILD_TYPE "Debug")
endif()

ADD_EXECUTABLE(${PROJECT_NAME} WIN32 # MACOSX_BUNDLE
  ${sources}
  ${header}
  ${moc_header}
  ${form}
  ${resource}
  ${moc_generated}
  ${forms_generated}
  ${rc_generated}
  )

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
  common
  statemachine
  gui
  ${QT_LIBRARIES}
  )
