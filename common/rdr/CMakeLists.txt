set(INSTALL_HEAD_FILES
    AESInStream.h
    AESOutStream.h
    BufferedInStream.h
    BufferedOutStream.h
    FdInStream.h
    FdOutStream.h
    FileInStream.h
    HexInStream.h
    HexOutStream.h
    RandomStream.h
    TLSException.h
    TLSInStream.h
    TLSOutStream.h
    ZlibInStream.h
    ZlibOutStream.h)
add_library(rdr OBJECT
  ${INSTALL_HEAD_FILES}
  AESInStream.cxx
  AESOutStream.cxx
  BufferedInStream.cxx
  BufferedOutStream.cxx
  FdInStream.cxx
  FdOutStream.cxx
  FileInStream.cxx
  HexInStream.cxx
  HexOutStream.cxx
  RandomStream.cxx
  TLSException.cxx
  TLSInStream.cxx
  TLSOutStream.cxx
  ZlibInStream.cxx
  ZlibOutStream.cxx)

target_include_directories(rdr PRIVATE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/common>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>)
target_include_directories(rdr SYSTEM PRIVATE ${ZLIB_INCLUDE_DIRS})
target_link_libraries(rdr core)
target_link_libraries(rdr ${ZLIB_LIBRARIES})

if(GNUTLS_FOUND)
  target_include_directories(rdr SYSTEM PRIVATE ${GNUTLS_INCLUDE_DIR})
  target_link_libraries(rdr ${GNUTLS_LIBRARIES})
endif()
if (NETTLE_FOUND)
  target_include_directories(rdr SYSTEM PRIVATE ${NETTLE_INCLUDE_DIRS})
  target_link_libraries(rdr ${NETTLE_LIBRARIES})
  target_link_directories(rdr PUBLIC ${NETTLE_LIBRARY_DIRS})
endif()
if(WIN32)
	target_link_libraries(rdr ws2_32)
endif()

if(UNIX)
  libtool_create_control_file(rdr)
endif()

# Set install head files
set_target_properties(rdr PROPERTIES
  PUBLIC_HEADER "${INSTALL_HEAD_FILES}")
# Install head files
INSTALL(TARGETS rdr
  EXPORT rdrConfig
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tigervnc/rdr
    COMPONENT Development
  INCLUDES DESTINATION tigervnc
  )
# Export cmake configure file
export(TARGETS rdr
  APPEND FILE ${CMAKE_CURRENT_BINARY_DIR}/rdrConfig.cmake
  )
install(EXPORT rdrConfig
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/tigervnc"
    COMPONENT Development
  )
