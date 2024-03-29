project(firebird_db_example)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${Boost_INCLUDE_DIR})

# set(QT_USE_QTMAIN ON)
set(QT_USE_QTSQL ON)
include(${QT_USE_FILE})

set(sources
  main.cpp
  Employee.cpp
  AutoFetchTableView.cpp
  WSqlQueryModel.cpp
  MainWindow.cpp
  Tables.cpp
  )

set(moc_header
  AutoFetchTableView.h
  WSqlQueryModel.h
  MainWindow.h
  )

set(header
  Employee.h
  Tables.h
  )

set(forms
  )

set(resources
  )

qt4_wrap_cpp(moc_generated ${moc_header})
qt4_wrap_ui(forms_generated ${forms})
qt4_add_resources(rc_generated ${resources})

add_executable(${PROJECT_NAME} # WIN32
  MACOSX_BUNDLE
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
  WaffleLibCommon
  WaffleLibDb
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  # COMPILE_DEFINITIONS _AFXDLL,UNICODE
  FOLDER "example"
  )

