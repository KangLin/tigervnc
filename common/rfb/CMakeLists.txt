set(INSTALL_HEAD_FILES
  AccessRights.h
  Blacklist.h
  Congestion.h
  CConnection.h
  CMsgHandler.h
  CMsgReader.h
  CMsgWriter.h
  CSecurityPlain.h
  CSecurityStack.h
  CSecurityVeNCrypt.h
  CSecurityVncAuth.h
  ClientParams.h
  ComparingUpdateTracker.h
  CopyRectDecoder.h
  Cursor.h
  DecodeManager.h
  Decoder.h
  d3des.h
  EncodeManager.h
  Encoder.h
  HextileDecoder.h
  HextileEncoder.h
  JpegCompressor.h
  JpegDecompressor.h
  KeyRemapper.h
  KeysymStr.h
  PixelBuffer.h
  PixelFormat.h
  RREEncoder.h
  RREDecoder.h
  RawDecoder.h
  RawEncoder.h
  SConnection.h
  SMsgHandler.h
  SMsgReader.h
  SMsgWriter.h
  ServerCore.h
  ServerParams.h
  Security.h
  SecurityServer.h
  SecurityClient.h
  SSecurityPlain.h
  SSecurityStack.h
  SSecurityVncAuth.h
  SSecurityVeNCrypt.h
  TightDecoder.h
  TightEncoder.h
  TightJPEGEncoder.h
  UpdateTracker.h
  VNCSConnectionST.h
  VNCServerST.h
  ZRLEEncoder.h
  ZRLEDecoder.h
  encodings.h
  obfuscate.h
  )
set(HEAD_FILES
  ${INSTALL_HEAD_FILES}
  )
add_library(rfb OBJECT
  ${HEAD_FILES}
  AccessRights.cxx
  Blacklist.cxx
  Congestion.cxx
  CConnection.cxx
  CMsgHandler.cxx
  CMsgReader.cxx
  CMsgWriter.cxx
  CSecurityPlain.cxx
  CSecurityStack.cxx
  CSecurityVeNCrypt.cxx
  CSecurityVncAuth.cxx
  ClientParams.cxx
  ComparingUpdateTracker.cxx
  CopyRectDecoder.cxx
  Cursor.cxx
  DecodeManager.cxx
  Decoder.cxx
  d3des.c
  EncodeManager.cxx
  Encoder.cxx
  HextileDecoder.cxx
  HextileEncoder.cxx
  JpegCompressor.cxx
  JpegDecompressor.cxx
  KeyRemapper.cxx
  KeysymStr.c
  PixelBuffer.cxx
  PixelFormat.cxx
  RREEncoder.cxx
  RREDecoder.cxx
  RawDecoder.cxx
  RawEncoder.cxx
  SConnection.cxx
  SMsgHandler.cxx
  SMsgReader.cxx
  SMsgWriter.cxx
  ServerCore.cxx
  ServerParams.cxx
  Security.cxx
  SecurityServer.cxx
  SecurityClient.cxx
  SSecurityPlain.cxx
  SSecurityStack.cxx
  SSecurityVncAuth.cxx
  SSecurityVeNCrypt.cxx
  TightDecoder.cxx
  TightEncoder.cxx
  TightJPEGEncoder.cxx
  UpdateTracker.cxx
  VNCSConnectionST.cxx
  VNCServerST.cxx
  ZRLEEncoder.cxx
  ZRLEDecoder.cxx
  encodings.cxx
  obfuscate.cxx)

target_include_directories(rfb PRIVATE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/common>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>)
target_include_directories(rfb SYSTEM PRIVATE ${JPEG_INCLUDE_DIR})
target_link_libraries(rfb core rdr network)
target_link_libraries(rfb ${JPEG_LIBRARIES} ${PIXMAN_LIBRARIES})

if(ENABLE_H264 AND NOT H264_LIBS STREQUAL "NONE")
  target_sources(rfb PRIVATE H264Decoder.cxx H264DecoderContext.cxx)
  if(H264_LIBS STREQUAL "LIBAV")
    target_sources(rfb PRIVATE H264LibavDecoderContext.cxx)
  elseif(H264_LIBS STREQUAL "WIN")
    target_sources(rfb PRIVATE H264WinDecoderContext.cxx)
  endif()
  target_include_directories(rfb SYSTEM PRIVATE ${H264_INCLUDE_DIRS})
  target_link_libraries(rfb ${H264_LIBRARIES})
  target_link_directories(rfb PUBLIC ${H264_LIBRARY_DIRS})
endif()

if(WIN32)
  target_include_directories(rfb PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/win>)
  target_sources(rfb PRIVATE WinPasswdValidator.cxx)
endif(WIN32)

if(MSVC)
  # undef min and max macro
  target_compile_definitions(rfb PRIVATE NOMINMAX)
endif()

if(UNIX AND NOT APPLE)
  target_sources(rfb PRIVATE UnixPasswordValidator.cxx pam.c)
  target_link_libraries(rfb ${PAM_LIBS})
endif()

if(GNUTLS_FOUND)
  target_sources(rfb PRIVATE CSecurityTLS.cxx SSecurityTLS.cxx)
  target_include_directories(rfb SYSTEM PRIVATE ${GNUTLS_INCLUDE_DIR})
  target_link_libraries(rfb ${GNUTLS_LIBRARIES})
  # FIXME: Hack to block it marking gnutls_free() as dllimport
  if(WIN32 AND BUILD_STATIC)
    target_compile_definitions(rfb PRIVATE GNUTLS_INTERNAL_BUILD)
  endif()
endif()

if (NETTLE_FOUND)
  target_sources(rfb PRIVATE CSecurityDH.cxx CSecurityMSLogonII.cxx
                 CSecurityRSAAES.cxx SSecurityRSAAES.cxx)
  target_include_directories(rfb SYSTEM PRIVATE ${NETTLE_INCLUDE_DIRS}
                             ${GMP_INCLUDE_DIRS})
  target_link_libraries(rfb ${HOGWEED_LIBRARIES}
                        ${NETTLE_LIBRARIES} ${GMP_LIBRARIES})
  target_link_directories(rfb PUBLIC ${HOGWEED_LIBRARY_DIRS}
                          ${NETTLE_LIBRARY_DIRS} ${GMP_LIBRARY_DIRS})
endif()

if(UNIX)
  libtool_create_control_file(rfb)
endif()

# Set install head files
set_target_properties(rfb PROPERTIES
  PUBLIC_HEADER "${INSTALL_HEAD_FILES}")
# Install head files
INSTALL(TARGETS rfb
  EXPORT rfbConfig
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tigervnc/rfb
    COMPONENT Development
  INCLUDES DESTINATION tigervnc
  )
# Export cmake configure file
export(TARGETS rfb
  APPEND FILE ${CMAKE_CURRENT_BINARY_DIR}/rfbConfig.cmake
  )
install(EXPORT rfbConfig
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/tigervnc"
    COMPONENT Development
  )
