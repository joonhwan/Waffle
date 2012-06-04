project(packet)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  )

set(QT_USE_QTNETWORK ON)
include(${QT_USE_FILE})

set(sources
  WPacketFixedLengthModel.cpp
  WPacketHeadTailModel.cpp
  WPacketHeaderBodyModel.cpp
  WPacketIo.cpp
  WPacketStream.cpp
  WSerialPort.cpp
  WSerialPortEnumerator.cpp
  WTcpSocket.cpp
  )

set(moc_header
  WPacket.h
  WPacketHeaderBodyModel.h
  WPacketIo.h
  WSerialPort.h
  WTcpHeaderBodySocket.h
  WTcpSocket.h
  )

set(header
  WPacketStream.h
  WSerialPortEnumerator.h
  WPacket.h
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
  qextserial
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  # COMPILE_DEFINITIONS _AFXDLL,UNICODE
  FOLDER "waffle"
  )

