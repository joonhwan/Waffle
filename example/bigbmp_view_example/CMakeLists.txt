project(bigbmp_view_example)

set(QT_USE_QTMAIN ON)
include(${QT_USE_FILE})

include_directories(
  ${VLD_INCLUDE_DIR}
  )

set(sources
  main.cpp
  mainwindow.cpp
  WBigBmpGraphicsView.cpp
  )

set(moc_header
  mainwindow.h
  WBigBmpGraphicsView.h
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

# ???
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
  WaffleLibCommon
  WaffleLibGui
  ${QT_LIBRARIES}
  ${VLD_LIBRARIES}
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  FOLDER "example"
  )
