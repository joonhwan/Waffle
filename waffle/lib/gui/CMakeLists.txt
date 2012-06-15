project(gui)

include(${QT_USE_FILE})
include_directories(
  ${GraphViz_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

include(qhexedit/qhexedit.cmake)

set(sources
  ${QHEXEDIT_SOURCES}
  graphviz/GVGraph.cpp
  WAutoLayoutDiagramScene.cpp
  WBigBmpGraphicsView.cpp
  WBigBmpImageFileLoader.cpp
  WBigBmpRenderThread.cpp
  WBigBmpThreadedGraphicsScene.cpp
  WDecoratedSimpleLabel.cpp
  WEnumComboBox.cpp
  WGraphicsArrowItem.cpp
  WGraphicsCachedImageItem.cpp
  WGraphicsUtil.cpp
  WGraphicsVectorNodeItem.cpp
  WGraphicsVectorNodeItemParent.cpp
  WGraphicsVectorRectItem.cpp
  WGraphicsView.cpp
  WHexEdit.cpp
  WLogView.cpp
  WMainWindow.cpp
  WSerialPortPanel.cpp
  # WSettingEditInfo.cpp
  WSettingExchangeableEditor.cpp
  WSettingsEditor.cpp
  WTransparentProgressBar.cpp

  UserInitArea.cpp
  )

set(moc_header
  ${QHEXEDIT_MOC_HEADER}
  WBigBmpGraphicsView.h
  WBigBmpImageFileLoader.h
  WBigBmpRenderThread.h
  WBigBmpThreadedGraphicsScene.h
  WDecoratedSimpleLabel.h
  WEnumComboBox.h
  WGraphicsView.h
  WHexEdit.h
  WLogView.h
  WMainWindow.h
  WSerialPortPanel.h
  WSettingExchangeableEditor.h
  WSettingsEditor.h
  WTransparentProgressBar.h

  UserInitArea.h
  )

set(header
  ${QHEXEDIT_HEADER}
  graphviz/GVGraph.h
  ${moc_header}
  WAutoLayoutDiagramScene.h
  WGraphicsArrowItem.h
  WGraphicsCachedImageItem.h
  WGraphicsUtil.h
  WGraphicsVectorNodeItem.h
  WGraphicsVectorNodeItemParent.h
  WGraphicsVectorRectItem.h
  # WSettingEditInfo.h
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
  ${GraphViz_LIBRARIES}
  qtpropertybrowser
  )

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  FOLDER "waffle"
  )