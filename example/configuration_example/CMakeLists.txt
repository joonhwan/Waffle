PROJECT(configuration_example)

# Set up the environment
FIND_PACKAGE(Qt4 REQUIRED)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# SET(QT_USE_QTMAIN ON)
INCLUDE(${QT_USE_FILE})

SET(sources
  AppConfig.cpp
  main.cpp
  LoginInfo.cpp
  )

SET(moc_header
  )

SET(header
  AppConfig.h
  LoginInfo.h
  )

SET(forms
  )

SET(resources
  )

QT4_WRAP_CPP(moc_generated ${moc_header})
QT4_WRAP_UI(forms_generated ${forms})
QT4_ADD_RESOURCES(rc_generated ${resources})

ADD_EXECUTABLE(${PROJECT_NAME} MACOSX_BUNDLE
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
  ${QT_LIBRARIES}
  WaffleLibCommon
  )

SET_TARGET_PROPERTIES(${PROJECT_NAME}
  PROPERTIES
  FOLDER "example"
  )