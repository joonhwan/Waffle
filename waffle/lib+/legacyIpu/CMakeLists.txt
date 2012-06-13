project(legacyIpu)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

include(${QT_USE_FILE})

set(sources
  )

set(moc_header
  )

set(header
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
  FOLDER "common"
  )

target_link_libraries(${PROJECT_NAME}
  image
  property
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  FOLDER "waffle+"
  )
