project(yaml)

set(${PROJECT_NAME}_CPP
  api.c
  dumper.c
  emitter.c
  loader.c
  parser.c
  reader.c
  scanner.c
  writer.c
  )

set(${PROJECT_NAME}_H
  config.h
  yaml.h
  yaml_private.h
  )

set(${PROJECT_NAME}_MOC_H
  )

set(${PROJECT_NAME}_QRC
  )

set(${PROJECT_NAME}_MOC_H_INCLUDED_BY_SOURCE
  )

qt4_wrap_cpp(${PROJECT_NAME}_MOC_CPP ${${PROJECT_NAME}_MOC_H})
qt4_add_resources(${PROJECT_NAME}_QRC_CPP ${${PROJECT_NAME}_QRC})

add_definitions(
  -D_CRT_SECURE_NO_WARNINGS
  -DYAML_DECLARE_STATIC
  )

add_library(${PROJECT_NAME}
  ${${PROJECT_NAME}_CPP}
  ${${PROJECT_NAME}_MOC_CPP}
  ${${PROJECT_NAME}_MOC_H}
  ${${PROJECT_NAME}_CPP_WITH_MOC}
  ${${PROJECT_NAME}_MOC_FROM_CPP}
  ${${PROJECT_NAME}_QRC_CPP}
  ${${PROJECT_NAME}_MOC_CPP_INCLUDED_BY_SOURCE}
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  FOLDER "externals"
  )

target_link_libraries(${PROJECT_NAME}
  )
