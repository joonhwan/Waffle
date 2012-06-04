project(communication_test_client)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../communication_test_server
  )

set(QT_USE_QTMAIN ON)
set(QT_USE_QTNETWORK ON)
include(${QT_USE_FILE})

set(sources
  main.cpp
  MainWindow.cpp
  TripPlanner.cpp
  TripPlanResultModel.cpp
  WSimpleTableModel.cpp
  WSimpleTableView.cpp
  WHistoryComboBox.cpp
  WHistoryDateTimeEdit.cpp
  )

set(moc_header
  MainWindow.h
  TripPlanner.h
  TripPlanResultModel.h
  WSimpleTableModel.h
  WSimpleTableView.h
  WHistoryComboBox.h
  WHistoryDateTimeEdit.h
  )

set(header
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
  common
  packet
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  # COMPILE_DEFINITIONS _AFXDLL,UNICODE
  FOLDER "example"
  )

