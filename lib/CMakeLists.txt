set(LIBRARY_NAME "searpc")

set(Header_Files
        "searpc-client.h"
        "searpc-named-pipe-transport.h"
        "searpc-server.h"
        "searpc-utils.h"
        "searpc.h"
)
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files
        "searpc-client.c"
        "searpc-named-pipe-transport.c"
        "searpc-server.c"
        "searpc-utils.c"
)
source_group("Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" "${LIBRARY_TYPE_FLAG}" "${Header_Files}" "${Source_Files}")

include(FindPkgConfig)
pkg_check_modules (GLIB2 glib-2.0)
include("../cmake/modules/FindJansson.cmake")
if (NOT JANSSON_FOUND)
    message(FATAL_ERROR "JANSSON required")
endif ()

target_include_directories(
        "${LIBRARY_NAME}"
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<INSTALL_INTERFACE:include>"
        "${GLIB2_STATIC_INCLUDE_DIRS}"
        "${JANSSON_INCLUDE_DIRS}"
)
target_link_libraries("${LIBRARY_NAME}" PUBLIC "${GLIB2_LINK_LIBRARIES}" "${JANSSON_LIBRARIES}")
set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

# Symbol exporter
include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_SOURCE_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

# install rules

# setup the version numbering
set_property(TARGET "${LIBRARY_NAME}" PROPERTY VERSION "1.0.0")
set_property(TARGET "${LIBRARY_NAME}" PROPERTY SOVERSION "1")

set(installable_libs "${LIBRARY_NAME}" "${EXEC_NAME}" "${PROJECT_NAME}_compiler_flags")
install(FILES "${Header_Files}" DESTINATION "include")

if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif ()
install(TARGETS ${installable_libs}
        DESTINATION "lib"
        EXPORT "${LIBRARY_NAME}Targets")
