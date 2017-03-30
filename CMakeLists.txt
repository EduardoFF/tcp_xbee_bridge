cmake_minimum_required (VERSION 2.6)
project (xbee_bridge)

#-------------To add a 3rd party library-----------
find_package(PkgConfig)
pkg_check_modules(Glog libglog)

link_directories(${Glog_LIBRARY_DIRS})
include_directories(${Glog_INCLUDE_DIRS})
#--------------------------------------------------

set(CMAKE_MODULE_PATH $ENV{CMAKE_MODULE_PATH})

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH

  "Single Directory for all executables"
  )

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/libs CACHE PATH

  "Single Directory for all dynamic Libraries"
  )

MESSAGE( STATUS "CMAKE_C_FLAGS: " ${CMAKE_C_FLAGS} )
MESSAGE( STATUS "CMAKE_C_FLAGS_DEBUG: " ${CMAKE_C_FLAGS_DEBUG} )
MESSAGE( STATUS "CMAKE_C_FLAGS_RELEASE: " ${CMAKE_C_FLAGS_RELEASE} )

MESSAGE( STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_C_FLAGS} )
MESSAGE( STATUS "CMAKE_CXX_FLAGS_DEBUG: " ${CMAKE_C_FLAGS_DEBUG} )
MESSAGE( STATUS "CMAKE_CXX_FLAGS_RELEASE: " ${CMAKE_C_FLAGS_RELEASE} )
include_directories( ${PROJECT_SOURCE_DIR} )

set(XBeeInterface_HEADERS
  xbee_interface.h
)

set(XBeeInterface_IMPLEMENTATIONS
  xbee_interface.cc
  )

add_library(tcpconn tcpconn.cc tcpacceptor.cpp tcpconnector.cpp tcpstream.cpp)

add_executable(xbee_bridge xbee_bridge.cc xbee_interface.cc)
target_link_libraries(xbee_bridge
  tcpconn
  glog
  pthread
  xbeep.a
  xbee.a
  )

install(TARGETS xbee_bridge
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )
