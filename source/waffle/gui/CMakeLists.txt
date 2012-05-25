project(gui)

include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(sources
  WLogView.cpp
  WSettingsEditor.cpp
  WSettingExchangeableEditor.cpp
  )

set(moc_header
  WLogView.h
  WSettingsEditor.h
  WSettingExchangeableEditor.h
  )

set(header
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

# set_source_files_properties(${moc_generated}
#   PROPERTIES HEADER_FILE_ONLY true
#   )

target_link_libraries(${PROJECT_NAME}
  ${QT_LIBRARIES}
  qtpropertybrowser
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  FOLDER "waffle"
  )