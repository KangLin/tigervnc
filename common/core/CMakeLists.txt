set(INSTALL_HEAD_FILES
  Configuration.h
  Exception.h
  Logger.h
  Mutex.h
  Region.h
  Timer.h
  Thread.h
  string.h
  time.h
  Rect.h
  xdgdirs.h)
set(HEAD_FILES
  ${INSTALL_HEAD_FILES}
  Logger_file.h
  Logger_stdio.h
  LogWriter.h)
add_library(core OBJECT
  ${HEAD_FILES}
  Configuration.cxx
  Exception.cxx
  Logger.cxx
  Logger_file.cxx
  Logger_stdio.cxx
  LogWriter.cxx
  Mutex.cxx
  Region.cxx
  Timer.cxx
  Thread.cxx
  string.cxx
  time.cxx
  xdgdirs.cxx)

target_include_directories(core PRIVATE
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/common>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>)
target_include_directories(core PRIVATE ${PIXMAN_INCLUDE_DIRS})
target_link_libraries(core ${PIXMAN_LIBRARIES})
target_link_directories(core PUBLIC ${PIXMAN_LIBRARY_DIRS})

if(UNIX)
  target_sources(core PRIVATE Logger_syslog.cxx)
  target_link_libraries(core pthread)
endif()

if(WIN32)
  target_link_libraries(core ws2_32)
endif()
if(MSVC)
  # undef min and max macro
  target_compile_definitions(core PUBLIC NOMINMAX)
endif()

if(UNIX)
  target_sources(core PRIVATE Logger_syslog.cxx)
endif()

if(UNIX)
  libtool_create_control_file(core)
endif()

# Set install head files
set_target_properties(core PROPERTIES
  PUBLIC_HEADER "${INSTALL_HEAD_FILES}")
# Install head files
INSTALL(TARGETS core
  EXPORT coreConfig
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tigervnc/core
    COMPONENT Development
  INCLUDES DESTINATION tigervnc
  )
# Export cmake configure file
export(TARGETS core
  APPEND FILE ${CMAKE_CURRENT_BINARY_DIR}/coreConfig.cmake
  )
install(EXPORT coreConfig
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/tigervnc"
    COMPONENT Development
  )
