project(statemachine_test)

set(QT_USE_QTMAIN ON)
include(${QT_USE_FILE})

set(sources
  myevents.cpp
  testerwindow.cpp
  main.cpp
  mystatemachine.cpp
  )

set(moc_header
  myeventids.h
  myevents.h
  mystatemachine.h
  testerwindow.h
  )

set(header
  ${moc_header}
  )

set(forms
  )

set(resources
  )

qt4_wrap_cpp(moc_generated ${moc_header})
qt4_wrap_ui(forms_generated ${forms})
qt4_add_resources(rc_generated ${resources})

if(CMAKE_HOST_WIN32)
set(CMAKE_BUILD_TYPE "Debug")
endif()

add_executable(${PROJECT_NAME} WIN32 # MACOSX_BUNDLE
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
  common
  statemachine
  gui
  ${QT_LIBRARIES}
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  FOLDER "example"
  )