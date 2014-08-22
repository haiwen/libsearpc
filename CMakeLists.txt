# CMake build script for the libsearpc project
#
# Building (out of source build):
# > mkdir build && cd build
# > cmake .. [-DSETTINGS=VALUE]
# > cmake --build .
#
# Install:
# > cmake --build .
if(EXISTS CMakeLists.txt)
  message(FATAL_ERROR
    "Looks like you are trying to run cmake from the base source directory.\n"
    "** RUNNING CMAKE FROM THE BASE DIRECTORY WILL NOT WORK **\n"
    "To Fix:\n"
    " 1. Remove the CMakeCache.txt file in this directory. ex: rm CMakeCache.txt\n"
    " 2. Create a build directory from here. ex: mkdir build\n"
    " 3. cd into that directory. ex: cd build\n"
    " 4. Run cmake from the build directory. ex: cmake ..\n"
    " 5. Run make from the build directory. ex: make\n"
    "Full paste-able example:\n"
    "( rm -f CMakeCache.txt; mkdir build; cd build; cmake ..; make )\n"
    )
endif()
cmake_minimum_required(VERSION 2.8)

# Add path for custom modules
set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
  "${CMAKE_SOURCE_DIR}/cmake/Modules/"
  )

project(libsearpc)

set(LIBSEARPC_VERSION_MAJOR 1)
set(LIBSEARPC_VERSION_MINOR 2)
set(LIBSEARPC_VERSION_PATCH 2)
set(LIBSEARPC_VERSION_STRING "${LIBSEARPC_VERSION_MAJOR}.${LIBSEARPC_VERSION_MINOR}.${LIBSEARPC_VERSION_PATCH}")

if (NOT PACKAGE_VERSION)
  set(PACKAGE_VERSION "${LIBSEARPC_VERSION_STRING}")
endif()

set(PACKAGE_NAME libsearpc)
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "freeplant@gmail.com")
set(PACKAGE_URL "http://seafile.com")

set(BUG_REPORT_EMAIL "${PACKAGE_BUGREPORT}" CACHE STRING
  "Default Email where bug reports are to be submitted.")
set(BUG_REPORT_URL "https://github.com/haiwen/seafile/issues" CACHE STRING
  "Default Url where bug reports are to be submitted.")

# Configure CPack
set(CPACK_PACKAGE_NAME ${PACKAGE_NAME})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "libsearpc")
set(CPACK_PACKAGE_VENDOR "Haiwen Inc.")
set(CPACK_PACKAGE_VERSION_MAJOR ${LIBSEARPC_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${LIBSEARPC_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${LIBSEARPC_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Lingtao Pan <freeplant@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "http://www.seafile.com")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libglib2.0 (>= 2.28.0) libjansson4 (>=2.1)")
set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
set(CPACK_RPM_PACKAGE_URL "http://www.seafile.com")
set(CPACK_RPM_PACKAGE_REQUIRES "glib2 jansson")
include(CPack)

# SET UP DEFAULT CMAKE_BUILD_TYPE
if( NOT CMAKE_CONFIGURATION_TYPES )
  # Build Debug by default
  if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
  endif ()
else()
  # Using a multi-configuration generator eg MSVC or Xcode
  # that uses CMAKE_CONFIGURATION_TYPES and not CMAKE_BUILD_TYPE
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

option(LIBSEARPC_BUILD_DEMO
  "Build libsearpc demo." OFF)
option(LIBSEARPC_BUILD_TESTS
  "Build libsearpc tests." ON)
option(LIBSEARPC_BUILD_PYTHON
  "Build libsearpc python binding." ON)
option(LIBSEARPC_BUILD_STATIC
  "Build static library" OFF)

if(LIBSEARPC_BUILD_STATIC)
  set(LIBSEARPC_LIBTYPE STATIC)
else()
  set(LIBSEARPC_LIBTYPE SHARED)
endif()

# Find Library Dependencies
find_package(GLIB2 2.26.0 REQUIRED)
find_package(JANSSON 2.2.1 REQUIRED)

# System Check
# Borrowed from llvm
include(config-ix)

# Verify that we can find a Python 2 interpreter. Python 3 is unsupported.
set(Python_ADDITIONAL_VERSIONS 2.7 2.6 2.5)
include(FindPythonInterp)
if(NOT PYTHONINTERP_FOUND)
  message(FATAL_ERROR
"Unable to find Python interpreter, required for searpc-codegen.
Please install Python or specify the PYTHON_EXECUTABLE CMake variable.")
endif()

# CONFIG.H
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# INSTALLATION PATHS
set(BIN_INSTALL_DIR bin CACHE PATH "Where to install binaries to.")
set(LIB_INSTALL_DIR lib CACHE PATH "Where to install libraries to.")
set(INCLUDE_INSTALL_DIR include CACHE PATH "Where to install headers to.")

# LIB
set(SRC_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
file(GLOB SRC_LIB_H ${SRC_LIB_DIR}/*.h)
set(SRC_INCLUDE_DIR ${SRC_LIB_DIR})
file(GLOB SRC_INCLUDE_H ${SRC_INCLUDE_DIR}/*.h)
include_directories(lib)
add_subdirectory(lib)
set(LIBSEARPC_INCLUDE_DIR ${SRC_INCLUDE_DIR})
set(LIBSEARPC_INCLUDE_DIRS ${LIBSEARPC_INCLUDE_DIR})
set(LIBSEARPC_LIBRARIES libsearpc)

# INCLUDE
#
install(FILES ${SRC_INCLUDE_H}
  DESTINATION ${INCLUDE_INSTALL_DIR}
  )


# DEMO
set(SRC_DEMO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/demo)
if(LIBSEARPC_BUILD_DEMO)
  add_subdirectory(demo)
endif(LIBSEARPC_BUILD_DEMO)

# PYTHON BINDING
if(LIBSEARPC_BUILD_PYTHON)
  include(FindPythonLibs)
  if(NOT PYTHONLIBS_FOUND OR NOT PYTHON_EXECUTABLE)
    message(SEND_ERROR "You need Python to build the python bindings")
  else()
    set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.cmake")
    set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")
    set(DEPS        "${CMAKE_CURRENT_SOURCE_DIR}/pysearpc/__init__.py")
    set(OUTPUT      "${CMAKE_CURRENT_BINARY_DIR}/build")

    configure_file(${SETUP_PY_IN} ${SETUP_PY})

    add_custom_command(
      OUTPUT ${OUTPUT}/timestamp
      COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY} build
      COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}/timestamp
      DEPENDS ${DEPS}
    )
    add_custom_target(target ALL DEPENDS ${OUTPUT}/timestamp)
    install(CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY} -q install --prefix=${CMAKE_INSTALL_PREFIX}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)")
  endif()
endif(LIBSEARPC_BUILD_PYTHON)

# Unit Tests
if(LIBSEARPC_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif(LIBSEARPC_BUILD_TESTS)
