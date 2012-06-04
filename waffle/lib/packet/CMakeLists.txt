project(packet)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(QT_USE_QTNETWORK ON)
set(QT_USE_QTMAIN ON)
include(${QT_USE_FILE})

set(sources
  WTcpPacketHeaderBodyModel.cpp
  WTcpSocket.cpp
  WTcpSocketStream.cpp
  )

set(moc_header
  WTcpHeaderBodySocket.h
  WTcpPacketHeaderBodyModel.h
  WTcpSocket.h
  )

set(header
  WTcpSocketPacket.h
  WTcpSocketStream.h
  )

set(forms
  )

set(resources
  )

qt4_wrap_cpp(moc_generated ${moc_header})
qt4_wrap_ui(forms_generated ${forms})
qt4_add_resources(rc_generated ${resources})

add_library(${PROJECT_NAME}
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
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  # COMPILE_DEFINITIONS _AFXDLL,UNICODE
  FOLDER "waffle"
  )
