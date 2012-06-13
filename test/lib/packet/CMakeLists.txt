enable_testing()

project(packet.test)

set(QT_USE_QTTEST ON)
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(QT_USE_QTMAIN ON)
include(${QT_USE_FILE})

set(sources
  dummyhardware.cpp
  main.cpp
  WPacketModelTest.cpp
  mock/WIODeviceMock.cpp
  )

set(moc_header
  dummyhardware.h
  WPacketModelTest.h
  mock/WIODeviceMock.h
  )

set(header
  Protocol.h
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
  packet
  )

add_test(${PROJECT_NAME}
  ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}
  )

add_custom_command(TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND ${PROJECT_NAME}
  COMMENT "testing ${PROJECT_NAME} ..."
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  # COMPILE_DEFINITIONS _AFXDLL,UNICODE
  FOLDER "test"
  )
