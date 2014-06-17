prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@/@BIN_INSTALL_DIR@
libdir=@CMAKE_INSTALL_PREFIX@/@LIB_INSTALL_DIR@
includedir=@CMAKE_INSTALL_PREFIX@/@INCLUDE_INSTALL_DIR@

Name: libsearpc
Description: Simple C rpc library
Version: @LIBSEARPC_VERSION_STRING@
Requires: @LIBSEARPC_PC_REQUIRES@
Libs.private: @LIBSEARPC_PC_LIBS@
Libs: -L${libdir} -lsearpc
Cflags: -I${includedir}
