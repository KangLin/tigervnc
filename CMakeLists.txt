#
# Setup
#

cmake_minimum_required(VERSION 2.8.11)
if(POLICY CMP0022)
    cmake_policy(SET CMP0022 OLD)
endif()

if(${CMAKE_VERSION} VERSION_GREATER "3.30.0")
    cmake_policy(SET CMP0057 NEW)
endif()

if(${CMAKE_VERSION} VERSION_LESS "3.4.0")
    message(WARNING "CMake 3.4.0 or newer is required to get correct default installation paths")
endif()

# Internal cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

include(CMakePackageConfigHelpers)
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
include(CheckCSourceRuns)

include(CMakeMacroLibtoolFile)

project(tigervnc)
set(VERSION 1.11.80)

# The RC version must always be four comma-separated numbers
set(RCVERSION 1,11,80,0)

# Installation paths
include(GNUInstallDirs)
set(CMAKE_INSTALL_UNITDIR "lib/systemd/system" CACHE PATH "systemd unit files (lib/systemd/system)")
if(IS_ABSOLUTE "${CMAKE_INSTALL_UNITDIR}")
    set(CMAKE_INSTALL_FULL_UNITDIR "${CMAKE_INSTALL_UNITDIR}")
else()
    set(CMAKE_INSTALL_FULL_UNITDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_UNITDIR}")
endif()

option(INSTALL_SYSTEMD_UNITS "Install TigerVNC systemd units" ON)

if(NOT BUILD_TIMESTAMP)
    STRING(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M" UTC)
endif()

# Default to optimised builds instead of debug ones. Our code has no bugs ;)
# (CMake makes it fairly easy to toggle this back to Debug if needed)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

message(STATUS "VERSION = ${VERSION}")
message(STATUS "BUILD_TIMESTAMP = ${BUILD_TIMESTAMP}")
add_definitions(-DBUILD_TIMESTAMP="${BUILD_TIMESTAMP}")

# We want to keep our asserts even in release builds so add NDEBUG
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -UNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -UNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -UNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -UNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -UNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -UNDEBUG")

# Make sure we get a sane C version
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")

if(MSVC)
    # This option is to enable the /MP switch for Visual Studio 2005 and above compilers
    OPTION(WIN32_USE_MP "Set to ON to build with the /MP option (Visual Studio 2005 and above)." ON)
    MARK_AS_ADVANCED(WIN32_USE_MP)
    IF(WIN32_USE_MP)
        add_compile_options(/MP)
    ENDIF(WIN32_USE_MP)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
else()
    # Tell the compiler to be stringent
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wformat=2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wformat=2")
endif()
# Make sure we catch these issues whilst developing
IF(CMAKE_BUILD_TYPE MATCHES Debug)
    IF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Werror=vla")
    ENDIF()
    IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Werror=vla")
    ENDIF()
ENDIF()
# clang doesn't support format_arg, which breaks this warning
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format-nonliteral -Wno-format-security")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-nonliteral -Wno-format-security")
endif()

option(ENABLE_ASAN "Enable address sanitizer support" OFF)
if(ENABLE_ASAN AND NOT WIN32 AND NOT APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

option(ENABLE_TSAN "Enable thread sanitizer support" OFF)
if(ENABLE_TSAN AND NOT WIN32 AND NOT APPLE AND CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
endif()

if(NOT DEFINED BUILD_WINVNC)
    set(BUILD_WINVNC 1)
endif()

# Minimum version is Windows 7
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    message(STATUS "64-bit build")
else()
    message(STATUS "32-bit build")
endif()

# Versions of CMake before 2.8.7 do not properly support resource compilation
# with MinGW.  Boo!
if(MINGW AND "${CMAKE_VERSION}" VERSION_LESS "2.8.7")
    if(NOT DEFINED RC)
        set(CMAKE_RC_COMPILER_INIT windres)
    else()
        set(CMAKE_RC_COMPILER_INIT ${RC})
    endif()
    enable_language(RC)
    message(STATUS "Resource compiler: ${CMAKE_RC_COMPILER}")
    set(CMAKE_RC_COMPILE_OBJECT
        "<CMAKE_RC_COMPILER> <FLAGS> <DEFINES> -o <OBJECT> --output-format=coff <SOURCE>")
endif()
        
# X11 stuff. It's in a if() so that we can say REQUIRED
if(UNIX AND NOT APPLE AND NOT ANDROID)
    find_package(X11 REQUIRED)
endif()

# Check for gettext
option(ENABLE_NLS "Enable translation of program messages" ON)
if(ENABLE_NLS)
    # Tools
    find_package(Gettext)
    
    # Gettext needs iconv
    find_package(Iconv)
    
    if(ICONV_FOUND)
        # Headers and libraries (copied from licq)
        set(GETTEXT_FOUND FALSE)
        
        find_path(GETTEXT_INCLUDE_DIR libintl.h)
        if(GETTEXT_INCLUDE_DIR)
            set(CMAKE_REQUIRED_LIBRARIES ${ICONV_LIBRARIES})
            set(CMAKE_REQUIRED_FLAGS -fno-builtin-dgettext)
            check_function_exists(dgettext LIBC_HAS_DGETTEXT)
            if(LIBC_HAS_DGETTEXT)
                set(GETTEXT_FOUND TRUE)
            else()
                find_library(LIBINTL_LIBRARY NAMES intl libintl)
                if(LIBINTL_LIBRARY)
                    check_library_exists(${LIBINTL_LIBRARY} "dgettext" "" LIBINTL_HAS_DGETTEXT)
                    if(LIBINTL_HAS_DGETTEXT)
                        set(GETTEXT_LIBRARIES ${LIBINTL_LIBRARY} ${ICONV_LIBRARIES})
                        set(GETTEXT_FOUND TRUE)
                    endif()
                endif()
            endif()
            set(CMAKE_REQUIRED_LIBRARIES)
            set(CMAKE_REQUIRED_FLAGS)
        endif()
    endif()
    
    if(NOT GETTEXT_FOUND OR NOT ICONV_FOUND)
        message(WARNING "Gettext NOT found.  Native Language Support disabled.")
        set(ENABLE_NLS 0)
    endif()
endif()

# Check for libjpeg
find_package(JPEG REQUIRED)

# Warn if it doesn't seem to be the accelerated libjpeg that's found
set(CMAKE_REQUIRED_LIBRARIES ${JPEG_LIBRARIES})
set(CMAKE_REQUIRED_FLAGS -I${JPEG_INCLUDE_DIR})

set(JPEG_TEST_SOURCE "\n
    #include <stdio.h>\n
    #include <jpeglib.h>\n
    int main(void) {\n
    struct jpeg_compress_struct cinfo;\n
    struct jpeg_error_mgr jerr;\n
    cinfo.err=jpeg_std_error(&jerr);\n
    jpeg_create_compress(&cinfo);\n
    cinfo.input_components = 3;\n
    jpeg_set_defaults(&cinfo);\n
    cinfo.in_color_space = JCS_EXT_RGB;\n
    jpeg_default_colorspace(&cinfo);\n
    return 0;\n
    }")

if(CMAKE_CROSSCOMPILING)
    check_c_source_compiles("${JPEG_TEST_SOURCE}" FOUND_LIBJPEG_TURBO)
else()
    check_c_source_runs("${JPEG_TEST_SOURCE}" FOUND_LIBJPEG_TURBO)
endif()

set(CMAKE_REQUIRED_LIBRARIES)
set(CMAKE_REQUIRED_FLAGS)
set(CMAKE_REQUIRED_DEFINITIONS)

if(NOT FOUND_LIBJPEG_TURBO)
    message(STATUS "WARNING: You are not using libjpeg-turbo. Performance will suffer.")
endif()

include_directories(${JPEG_INCLUDE_DIR})

option(BUILD_JAVA "Build Java version of the TigerVNC Viewer" FALSE)
if(BUILD_JAVA)
    add_subdirectory(java)
endif()

option(BUILD_VIEWER "Build TigerVNC viewer" OFF)
if(BUILD_VIEWER)
	# Check for FLTK
	set(FLTK_SKIP_FLUID TRUE)
	set(FLTK_SKIP_OPENGL TRUE)
	set(FLTK_SKIP_FORMS TRUE)
	find_package(FLTK REQUIRED)

	if(UNIX AND NOT APPLE)
	  # No proper handling for extra X11 libs that FLTK might need...
	  if(X11_Xft_FOUND)
	    # Xft headers include references to fontconfig, so we need
	    # to link to that as well
	    find_library(FONTCONFIG_LIB fontconfig)
	    set(FLTK_LIBRARIES ${FLTK_LIBRARIES} ${X11_Xft_LIB} ${FONTCONFIG_LIB})
	  endif()
	  if(X11_Xinerama_FOUND)
	    set(FLTK_LIBRARIES ${FLTK_LIBRARIES} ${X11_Xinerama_LIB})
	  endif()
	  if(X11_Xfixes_FOUND)
	    set(FLTK_LIBRARIES ${FLTK_LIBRARIES} ${X11_Xfixes_LIB})
	  endif()
	  if(X11_Xcursor_FOUND)
	    set(FLTK_LIBRARIES ${FLTK_LIBRARIES} ${X11_Xcursor_LIB})
	  endif()
	  if(X11_Xrender_FOUND)
	    set(FLTK_LIBRARIES ${FLTK_LIBRARIES} ${X11_Xrender_LIB})
	  endif()
	endif()
endif()

# Check for GNUTLS library
option(ENABLE_GNUTLS "Enable protocol encryption and advanced authentication" ON)
if(ENABLE_GNUTLS)
  find_package(GnuTLS)
  if (GNUTLS_FOUND)
    include_directories(${GNUTLS_INCLUDE_DIR})
    add_definitions("-DHAVE_GNUTLS")
    add_definitions(${GNUTLS_DEFINITIONS})
  endif()
endif()

# Check for PAM library
if(UNIX AND NOT APPLE AND NOT ANDROID)
    check_include_files(security/pam_appl.h HAVE_PAM_H)
    set(CMAKE_REQUIRED_LIBRARIES -lpam)
    check_function_exists(pam_start HAVE_PAM_START)
    set(CMAKE_REQUIRED_LIBRARIES)
    if(HAVE_PAM_H AND HAVE_PAM_START)
        set(PAM_LIBS pam)
    else()
        message(FATAL_ERROR "Could not find PAM development files")
    endif()
endif()

# Generate config.h and make sure the source finds it
configure_file(config.h.in config.h)
add_definitions(-DHAVE_CONFIG_H)
include_directories(${CMAKE_BINARY_DIR})

include(cmake/StaticBuild.cmake)

add_subdirectory(common)

option(BUILD_SERVER "Build TigerVNC server" OFF)
if(BUILD_SERVER)
    if(WIN32)
        add_subdirectory(win)
    else()
        # No interest in building x related parts on Apple
        if(NOT APPLE)
            add_subdirectory(unix)
        endif()
    endif()
endif()

if(ENABLE_NLS)
    add_subdirectory(po)
endif()

if(BUILD_VIEWER)
    if(FLTK_FOUND)
        add_subdirectory(vncviewer)
        add_subdirectory(media)
    else()
        message(WARNING "Build TigerVNC viewer must need FLTK")
    endif()
endif()

option(BUILD_TESTS "Build TigerVNC tests" OFF)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(BUILD_VIEWER)
  add_subdirectory(release)
endif()

add_custom_target(install-runtime
    COMMAND
    "${CMAKE_COMMAND}" -DCMAKE_INSTALL_COMPONENT=Runtime
    -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake"
    )

# uninstall
configure_file("${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "cmake_uninstall.cmake" IMMEDIATE @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P cmake_uninstall.cmake)
