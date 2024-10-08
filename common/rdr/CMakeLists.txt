# Check for zlib
find_package(ZLIB REQUIRED)

set(TIGERVNC_INCLUDE_DIR ${TIGERVNC_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    PARENT_SCOPE)

set(TIGERVNC_SOURCES ${TIGERVNC_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/AESInStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/AESOutStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/Exception.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/FdInStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/FdOutStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/FileInStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/HexInStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/HexOutStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/BufferedInStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/BufferedOutStream.cxx    
    ${CMAKE_CURRENT_SOURCE_DIR}/RandomStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/TLSException.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/TLSInStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/TLSOutStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/ZlibInStream.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/ZlibOutStream.cxx
    PARENT_SCOPE)

set(RDR_HEADS
    ${CMAKE_CURRENT_SOURCE_DIR}/AESInStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/AESOutStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Exception.h
    ${CMAKE_CURRENT_SOURCE_DIR}/FdInStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/FdOutStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/FileInStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/HexInStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/HexOutStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/InStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/BufferedInStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/BufferedOutStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/OutStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RandomStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TLSException.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TLSInStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TLSOutStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ZlibInStream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ZlibOutStream.h
    )

set(RDR_LIBRARIES ${ZLIB_LIBRARIES})
if(GNUTLS_FOUND)
    set(RDR_LIBRARIES ${RDR_LIBRARIES} ${GNUTLS_LIBRARIES})
endif()
if(WIN32)
    set(RDR_LIBRARIES ${RDR_LIBRARIES} ws2_32)
endif()


set(TIGERVNC_HEADS ${TIGERVNC_HEADS}
    ${RDR_HEADS}
    PARENT_SCOPE)

set(TIGERVNC_LIBS ${TIGERVNC_LIBS}
    ${RDR_LIBRARIES}
    PARENT_SCOPE)

INSTALL(FILES ${RDR_HEADS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/rdr
    )
