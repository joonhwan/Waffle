set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_ADDITIONAL_VERSIONS "1.49.0")
find_package(Boost 1.49.0 REQUIRED)

# include_directories(${Boost_INCLUDE_DIRS} COMPONENTS filesystem)
include_directories(${Boost_INCLUDE_DIRS})
set(BOOST_LIBRARIES ${Boost_LIBRARIES})

link_directories(${Boost_INCLUDE_DIRS}/lib/${PLATFORM_SUFFIX})