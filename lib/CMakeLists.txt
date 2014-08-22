################################
# LIB
################################

include_directories(${GLIB2_INCLUDE_DIR})
include_directories(${JANSSON_INCLUDE_DIR})
add_definitions(-DG_LOG_DOMAIN=\"Searpc\")

set(SRC_LIB
  searpc-client.c
  searpc-server.c
  searpc-utils.c
  ${platform_specific_sources}
  )

# Add libsearpc library
add_library(libsearpc ${LIBSEARPC_LIBTYPE} ${SRC_LIB_H} ${SRC_LIB})
target_link_libraries(libsearpc
  ${GLIB2_LIBRARIES} gobject-2.0 gio-2.0
  ${JANSSON_LIBRARIES}
  )
set_target_properties(libsearpc PROPERTIES OUTPUT_NAME libsearpc)
set_target_properties(libsearpc PROPERTIES PREFIX "")
set_target_properties(libsearpc PROPERTIES SOVERSION ${LIBSEARPC_VERSION_MAJOR})

# Install library
install(TARGETS libsearpc
  RUNTIME DESTINATION ${BIN_INSTALL_DIR}
  LIBRARY DESTINATION ${LIB_INSTALL_DIR}
  ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
  )

# Install searpc-codegen.py
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/searpc-codegen.py
  PERMISSIONS WORLD_EXECUTE WORLD_READ GROUP_READ GROUP_EXECUTE OWNER_EXECUTE OWNER_WRITE OWNER_READ
  DESTINATION ${BIN_INSTALL_DIR}
  )

# PKG-CONFIG FILE
set(LIBSEARPC_PC_REQUIRES "")
set(LIBSEARPC_PC_REQUIRES "${LIBSEARPC_PC_REQUIRES} gobject-2.0 gio-2.0 jansson")
set(LIBSEARPC_PC_LIBS "")
set(LIBSEARPC_PC_LIBS "${LIBSEARPC_PC_LIBS} -lglib-2.0 -ljansson")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libsearpc.pc.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/libsearpc.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libsearpc.pc
  DESTINATION ${LIB_INSTALL_DIR}/pkgconfig
  )

