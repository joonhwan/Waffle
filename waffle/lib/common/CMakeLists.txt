project(WaffleLibCommon)

if(WIN32 AND NOT BUILD_64)
  set(QT_USE_QAXCONTAINER ON)
endif()
include(${QT_USE_FILE})
include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  )

include(config/config.cmake)
include(global/global.cmake)
include(logger/logger.cmake)

set(sources
  ${config_sources}
  ${global_sources}
  ${logger_sources}
  )

set(moc_headers
  ${config_moc_headers}
  ${global_moc_headers}
  ${logger_moc_headers}
  )

set(headers
  ${config_headers}
  ${global_headers}
  ${logger_headers}
  )

if(WIN32 AND NOT BUILD_64)
  include(activex/activex.cmake)
  set(sources ${sources} ${activex_sources})
  set(moc_headers ${moc_headers} ${activex_moc_headers})
  set(headers ${headers} ${activex_headers})
endif()

set(forms
  )

set(resources
  )

qt4_wrap_cpp(moc_generated ${moc_headers})
qt4_wrap_ui(forms_generated ${forms})
qt4_add_resources(rc_generated ${resources})

add_library(${PROJECT_NAME} STATIC
  ${sources}
  ${headers}
  ${moc_headers}
  ${form}
  ${resource}
  ${moc_generated}
  ${forms_generated}
  ${rc_generated}
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  FOLDER "waffle"
  )

target_link_libraries(${PROJECT_NAME}
  ${QT_LIBRARIES}
  lua
  )
  
# set_source_files_properties(${moc_generated}
#   PROPERTIES HEADER_FILE_ONLY true
#   )
