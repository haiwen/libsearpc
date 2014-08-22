################################
# Unit Tests
################################

include_directories(${GLIB2_INCLUDE_DIR})
include_directories(${JANSSON_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Generate rpc-table
set(SRC_TEST_RPCTABLE_H
  ${CMAKE_CURRENT_SOURCE_DIR}/searpc-signature.h
  ${CMAKE_CURRENT_SOURCE_DIR}/searpc-marshal.h
  )
add_custom_command(
  PRE_BUILD
  OUTPUT ${SRC_TEST_RPCTABLE_H}
  COMMAND ${PYTHON_EXECUTABLE} ../lib/searpc-codegen.py rpc_table.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

# Generate clar.suite
set(SRC_CLAR_SUITE
  ${CMAKE_CURRENT_SOURCE_DIR}/clar.suite
  )
add_custom_command(
  PRE_BUILD
  OUTPUT ${SRC_CLAR_SUITE}
  COMMAND ${PYTHON_EXECUTABLE} generate.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

# Add test-searpc file
add_executable(searpc-test ${SRC_TEST_RPCTABLE_H}
  ${SRC_CLAR_SUITE}
  main.c clar.c
  searpc.c
  )
target_link_libraries(searpc-test
  ${GLIB2_LIBRARIES} gobject-2.0 gio-2.0
  ${JANSSON_LIBRARIES}
  libsearpc
  )
add_test(searpc-test searpc-test)
