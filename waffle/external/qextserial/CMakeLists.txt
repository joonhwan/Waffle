project(qextserial)

include(${QT_USE_FILE})

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  )

set(${PROJECT_NAME}_CPP
  qextserialport.cpp
  qextserialenumerator.cpp
  # win32
  qextserialport_win.cpp
  qextserialenumerator_win.cpp
  )

set(${PROJECT_NAME}_H
  qextserialport_global.h
  )

set(${PROJECT_NAME}_MOC_H
  qextserialenumerator.h
  )

set(${PROJECT_NAME}_QRC
  )

set(${PROJECT_NAME}_MOC_H_INCLUDED_BY_SOURCE
  qextserialport.h
  )

set(${PROJECT_NAME}_MOC_CPP_INCLUDED_BY_SOURCE "")
foreach(moc_from_cpp ${${PROJECT_NAME}_MOC_H_INCLUDED_BY_SOURCE})
  get_filename_component(moc_from_cpp_filetitle ${moc_from_cpp} NAME_WE)
  set(target_moc_name "moc_${moc_from_cpp_filetitle}.cpp")
  # message("generating ${target_moc_name}")
  qt4_generate_moc(${moc_from_cpp} ${target_moc_name})
  set(${PROJECT_NAME}_MOC_CPP_INCLUDED_BY_SOURCE ${${PROJECT_NAME}_MOC_CPP_INCLUDED_BY_SOURCE} ${target_moc_name})
endforeach(moc_from_cpp)

qt4_wrap_cpp(${PROJECT_NAME}_MOC_CPP ${${PROJECT_NAME}_MOC_H})
qt4_add_resources(${PROJECT_NAME}_QRC_CPP ${${PROJECT_NAME}_QRC})

add_definitions(
  -D_CRT_SECURE_NO_WARNINGS
  )

add_library(${PROJECT_NAME}
  ${${PROJECT_NAME}_CPP}
  ${${PROJECT_NAME}_MOC_CPP}
  ${${PROJECT_NAME}_MOC_H}
  ${${PROJECT_NAME}_CPP_WITH_MOC}
  ${${PROJECT_NAME}_MOC_FROM_CPP}
  # ${${PROJECT_NAME}_MOC_CPP_INCLUDED_BY_SOURCE}
  ${${PROJECT_NAME}_QRC_CPP}
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  FOLDER "externals"
  )

target_link_libraries(${PROJECT_NAME}
  # ${CMAKE_CURRENT_SOURCE_DIR}/ib_util_ms.lib
  advapi32.lib # for win32 service api
  setupapi.lib
  )
