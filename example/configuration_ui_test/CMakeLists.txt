project(configuration_ui_test)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# SET(QT_USE_QTMAIN ON)
include(${QT_USE_FILE})

set(sources
  AppConfig.cpp
  main.cpp
  LoginInfo.cpp
  )

set(moc_header
  AppConfigEnums.h
  )

set(header
  AppConfig.h
  LoginInfo.h
  )

set(forms
  )

set(resources
  )

qt4_wrap_cpp(moc_generated ${moc_header})
qt4_wrap_ui(forms_generated ${forms})
qt4_add_resources(rc_generated ${resources})

add_executable(${PROJECT_NAME} MACOSX_BUNDLE
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
  gui
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  FOLDER "example"
  )