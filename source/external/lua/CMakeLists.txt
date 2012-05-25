project(lua)

set(${PROJECT_NAME}_CPP
  lapi.c
  lauxlib.c
  lbaselib.c
  lcode.c
  ldblib.c
  ldebug.c
  ldo.c
  ldump.c
  lfunc.c
  lgc.c
  linit.c
  liolib.c
  llex.c
  lmathlib.c
  lmem.c
  loadlib.c
  lobject.c
  lopcodes.c
  loslib.c
  lparser.c
  lstate.c
  lstring.c
  lstrlib.c
  ltable.c
  ltablib.c
  ltm.c
  lundump.c
  lvm.c
  lzio.c
  )

set(${PROJECT_NAME}_H
  lapi.h
  lauxlib.h
  lcode.h
  ldo.h
  lfunc.h
  lgc.h
  llex.h
  lmem.h
  lobject.h
  lopcodes.h
  lstate.h
  lstring.h
  ltable.h
  ltm.h
  lua.h
  lua.hpp
  luaconf.h
  lualib.h
  luna.h
  lundump.h
  lvm.h
  lzio.h
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

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
  )
