project(common)

include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(sources
  config/WLuaHelpers.cpp
  config/WLuaSettings.cpp
  config/WSettingExchangeable.cpp
  config/WSettingsHelper.cpp
  global/WGlobal.cpp
  global/WRandom.cpp
  global/wnamespace.cpp
  global/WUacHelper.cpp
  logger/wlogdebugoutput.cpp
  logger/wlogevent.cpp
  logger/wlogfileoutput.cpp
  logger/wlogger.cpp
  logger/wlogguioutput.cpp
  logger/wlogoutput.cpp
  )

set(moc_header
  config/WLuaSettings.h
  logger/wlogguioutput.h
  global/wnamespace.h
  )

set(header
  config/WLuaHelpers.h
  config/WSettingConfigArray.h
  config/WSettingExchangeTraits.h
  config/WSettingExchangeable.h
  config/WSettingExchanger.h
  config/WSettingsHelper.h
  global/WException.h
  global/WGlobal.h
  global/WRandom.h
  global/WUacHelper.h
  logger/wlogdebugoutput.h
  logger/wlogevent.h
  logger/wlogfileoutput.h
  logger/wlogger.h
  logger/wlogoutput.h
  ${moc_header}
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
  FOLDER "waffle"
  )

target_link_libraries(${PROJECT_NAME}
  ${QT_LIBRARIES}
  lua
  )
  
# set_source_files_properties(${moc_generated}
#   PROPERTIES HEADER_FILE_ONLY true
#   )
