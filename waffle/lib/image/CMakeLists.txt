project(WaffleLibImage)

include_directories(
  ${OpenCV_INCLUDE_DIR}
  ${Ipp_INCLUDE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

include(${QT_USE_FILE})

set(sources
  WHistogram.cpp
  WImageBuffer.cpp
  WImageBufferT.cpp
  WImageProcessor.cpp
  )

set(moc_header
  )

set(header
  ippi.hpp
  ippi_ext.hpp
  Vld.h
  WConstImageT.h
  WHistogram.h
  WImageBuffer.h
  WImageBuffer.inl
  WImageBufferT.h
  WImageBufferT.inl
  WImageBufferTraits.h
  WImageProcessor.h
  WImageProcessorMacros.h
  WImageT.h
  WImageUtils.h
  WIppKernel.h
  WLookupTable.h
  WLookupTableTraits.h
  WPixel.h
  WStopwatch.h
  ippi.hpp
)

set(forms
  )

set(resources
  )

qt4_wrap_cpp(moc_generated ${moc_header})
qt4_wrap_ui(forms_generated ${forms})
qt4_add_resources(rc_generated ${resources})

add_library(${PROJECT_NAME} STATIC
  ${sources}
  ${header}
  ${moc_header}
  ${form}
  ${resource}
  ${moc_generated}
  ${forms_generated}
  ${rc_generated}
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  # COMPILE_DEFINITIONS _AFXDLL,UNICODE
  FOLDER "waffle"
  )

target_link_libraries(${PROJECT_NAME}
  ${IPP_LIBRARIES}
  ${VLD_LIBRARIES}
  ${OpenCV_LIBRARIES}
  )

