project(image_test)

include_directories(
  ${OpenCV_INCLUDE_DIR}
  ${Ipp_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

set(QT_USE_QTMAIN ON)
include(${QT_USE_FILE})

set(sources
  main.cpp
  ColorIcon.cpp
  MainWindow.cpp
  WCrossSplitter.cpp
  WDecoratedSimpleLabel.cpp
  WIntensityProfileWidget.cpp
  WImageViewer.cpp
  WImageViewerPixelItem.cpp
  )

set(moc_header
  MainWindow.h
  WCrossSplitter.h
  WDecoratedSimpleLabel.h
  WIntensityProfileWidget.h
  WImageViewer.h
  )

set(header
  ColorIcon.h
  WImageViewerPixelItem.h
  )

set(forms
  )

set(resources
  )

qt4_wrap_cpp(moc_generated ${moc_header})
qt4_wrap_ui(forms_generated ${forms})
qt4_add_resources(rc_generated ${resources})

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
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
  image
  property
  algo
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  # COMPILE_DEFINITIONS _AFXDLL,UNICODE
  FOLDER "example"
  )

