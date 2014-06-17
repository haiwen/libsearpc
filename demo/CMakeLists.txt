################################
# DEMO
################################

include_directories(${GLIB2_INCLUDE_DIR})
include_directories(${JANSSON_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Generate rpc-table
set(SRC_DEMO_RPCTABLE_H
  ${CMAKE_CURRENT_SOURCE_DIR}/searpc-signature.h
  ${CMAKE_CURRENT_SOURCE_DIR}/searpc-marshal.h
  )
add_custom_command(
  PRE_BUILD
  OUTPUT  ${SRC_DEMO_RPCTABLE_H}
  COMMAND ${PYTHON_EXECUTABLE} ../lib/searpc-codegen.py rpc_table.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

# Add searpc-demo-server
add_executable(searpc-demo-server
  ${SRC_DEMO_RPCTABLE_H}
  test-object.c
  searpc-demo-server.c
  )
target_link_libraries(searpc-demo-server
  ${GLIB2_LIBRARIES} gobject-2.0 gio-2.0
  ${JANSSON_LIBRARIES}
  libsearpc
  )
if(PURE_WINDOWS)
target_link_libraries(searpc-demo-server ws2_32)
endif(PURE_WINDOWS)

# Add searpc-demo-client
add_executable(searpc-demo-client
  ${SRC_DEMO_RPCTABLE_H}
  test-object.c
  searpc-demo-client.c
  )
target_link_libraries(searpc-demo-client
  ${GLIB2_LIBRARIES} gobject-2.0 gio-2.0
  ${JANSSON_LIBRARIES}
  libsearpc
  )
if(PURE_WINDOWS)
target_link_libraries(searpc-demo-client ws2_32)
endif(PURE_WINDOWS)

# Add searpc-async-client
add_executable(searpc-async-client
  ${SRC_DEMO_RPCTABLE_H}
  demo-async-client.c
  )
target_link_libraries(searpc-async-client
  ${GLIB2_LIBRARIES} gobject-2.0 gio-2.0
  ${JANSSON_LIBRARIES}
  libsearpc
  )
if(PURE_WINDOWS)
target_link_libraries(searpc-async-client ws2_32)
endif(PURE_WINDOWS)

