set(INSTALL_HEAD_FILES
  Socket.h
  TcpSocket.h)

add_library(network OBJECT
  ${INSTALL_HEAD_FILES}
  Socket.cxx
  TcpSocket.cxx)

if(NOT WIN32)
  target_sources(network PRIVATE UnixSocket.cxx)
endif()

target_include_directories(network PRIVATE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/common>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>)
target_link_libraries(network core rdr)

if(WIN32)
	target_link_libraries(network ws2_32)
endif()

if(UNIX)
  libtool_create_control_file(network)
endif()

# Set install head files
set_target_properties(network PROPERTIES
  PUBLIC_HEADER "${INSTALL_HEAD_FILES}")
# Install head files
INSTALL(TARGETS network
  EXPORT networkConfig
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tigervnc/network
    COMPONENT Development
  INCLUDES DESTINATION tigervnc
  )
# Export cmake configure file
export(TARGETS network
  APPEND FILE ${CMAKE_CURRENT_BINARY_DIR}/networkConfig.cmake
  )
install(EXPORT networkConfig
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/tigervnc"
    COMPONENT Development
  )
