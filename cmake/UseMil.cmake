find_package(Mil REQUIRED)

include_directories(${MIL_H_DIR})

set(MIL_LIBRARIES "general;${MIL_LIB_DIR}/mil.lib")
