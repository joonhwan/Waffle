project(flamerobin)

set(QT_USE_QTSQL ON)
set(QT_USE_QTXML ON)
include(${QT_USE_FILE})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(${PROJECT_NAME}_CPP
  frutils.cpp
  flamerobin-port-helper.cpp
  flamerobin-ui-wrapper.cpp

  metadata/column.cpp
  metadata/constraints.cpp
  metadata/CreateDDLVisitor.cpp
  metadata/database.cpp
  metadata/domain.cpp
  metadata/exception.cpp
  metadata/function.cpp
  metadata/generator.cpp
  metadata/Index.cpp
  metadata/metadataitem.cpp
  metadata/MetadataItemCreateStatementVisitor.cpp
  metadata/MetadataItemDescriptionVisitor.cpp
  metadata/MetadataItemURIHandlerHelper.cpp
  metadata/MetadataItemVisitor.cpp
  metadata/MetadataTemplateCmdHandler.cpp
  metadata/MetadataTemplateManager.cpp
  metadata/parameter.cpp
  metadata/privilege.cpp
  metadata/procedure.cpp
  metadata/relation.cpp
  metadata/role.cpp
  metadata/root.cpp
  metadata/server.cpp
  metadata/table.cpp
  metadata/trigger.cpp
  metadata/User.cpp
  metadata/view.cpp

  # core/ArtProvider.cpp
  # core/CodeTemplateProcessor.cpp
  core/FRError.cpp
  core/Observer.cpp
  core/ProgressIndicator.cpp
  core/StringUtils.cpp
  core/Subject.cpp
  core/TemplateProcessor.cpp
  core/URIProcessor.cpp
  core/Visitor.cpp

  engine/MetadataLoader.cpp

  ibpp/array.cpp
  ibpp/blob.cpp
  ibpp/database.cpp
  ibpp/date.cpp
  ibpp/dbkey.cpp
  ibpp/events.cpp
  ibpp/exception.cpp
  ibpp/row.cpp
  ibpp/service.cpp
  ibpp/statement.cpp
  ibpp/time.cpp
  ibpp/transaction.cpp
  ibpp/user.cpp
  ibpp/_dpb.cpp
  ibpp/_ibpp.cpp
  ibpp/_ibs.cpp
  ibpp/_rb.cpp
  ibpp/_spb.cpp
  ibpp/_tpb.cpp

  sql/Identifier.cpp
  sql/IncompleteStatement.cpp
  sql/MultiStatement.cpp
  sql/SelectStatement.cpp
  sql/SqlStatement.cpp
  sql/SqlTokenizer.cpp
  sql/StatementBuilder.cpp

  config/Config.cpp
  config/DatabaseConfig.cpp
  )

set(${PROJECT_NAME}_H
  frutils.h
  flamerobin-port-helper.h
  flamerobin-ui-wrapper.h

  metadata/collection.h
  metadata/column.h
  metadata/constraints.h
  metadata/CreateDDLVisitor.h
  metadata/database.h
  metadata/domain.h
  metadata/exception.h
  metadata/function.h
  metadata/generator.h
  metadata/Index.h
  metadata/MetadataClasses.h
  metadata/metadataitem.h
  metadata/MetadataItemCreateStatementVisitor.h
  metadata/MetadataItemDescriptionVisitor.h
  metadata/MetadataItemURIHandlerHelper.h
  metadata/MetadataItemVisitor.h
  metadata/MetadataTemplateManager.h
  metadata/parameter.h
  metadata/privilege.h
  metadata/procedure.h
  metadata/relation.h
  metadata/role.h
  metadata/root.h
  metadata/server.h
  metadata/table.h
  metadata/trigger.h
  metadata/User.h
  metadata/view.h

  # core/ArtProvider.h
  # core/CodeTemplateProcessor.h
  core/FRError.h
  core/ObjectWithHandle.h
  core/Observer.h
  core/ProcessableObject.h
  core/ProgressIndicator.h
  core/StringUtils.h
  core/Subject.h
  core/TemplateProcessor.h
  core/URIProcessor.h
  core/Visitor.h

  engine/MetadataLoader.h

  ibpp/ibase.h
  ibpp/iberror.h
  ibpp/ibpp.h
  ibpp/_ibpp.h

  sql/Identifier.h
  sql/IncompleteStatement.h
  sql/MultiStatement.h
  sql/SelectStatement.h
  sql/SqlStatement.h
  sql/SqlTokenizer.h
  sql/StatementBuilder.h

  config/Config.h
  config/DatabaseConfig.h
  )

set(${PROJECT_NAME}_MOC_H
  )

set(${PROJECT_NAME}_QRC
  )

qt4_wrap_cpp(${PROJECT_NAME}_MOC_CPP ${${PROJECT_NAME}_MOC_H})
qt4_add_resources(${PROJECT_NAME}_QRC_CPP ${${PROJECT_NAME}_QRC})

add_definitions(
  -D_CRT_SECURE_NO_WARNINGS
  -DIBPP_WINDOWS
  )

add_library(${PROJECT_NAME}
  ${${PROJECT_NAME}_CPP}
  ${${PROJECT_NAME}_MOC_CPP}
  ${${PROJECT_NAME}_MOC_H}
  ${${PROJECT_NAME}_CPP_WITH_MOC}
  ${${PROJECT_NAME}_MOC_FROM_CPP}
  ${${PROJECT_NAME}_QRC_CPP}
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  FOLDER "externals"
  )

target_link_libraries(${PROJECT_NAME}
  firebird
  # ${CMAKE_CURRENT_SOURCE_DIR}/ib_util_ms.lib
  advapi32.lib # for win32 service api
  ${QT_LIBRARIES}
  )
