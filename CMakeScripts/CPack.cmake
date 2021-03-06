# This file reconfigures variables at CPack time.
#
# It is called by
# https://cmake.org/cmake/help/latest/module/CPack.html#variable:CPACK_PROJECT_CONFIG_FILE
#

#used to get cmake-time variables at cpack-time
set(INKSCAPE_VERSION_SUFFIX @INKSCAPE_VERSION_SUFFIX@)
set(CMAKE_SOURCE_DIR @CMAKE_SOURCE_DIR@)

if(CPACK_GENERATOR STREQUAL "NSIS")
    set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/packaging/nsis/header.bmp") # TODO: this is odd - isn't there a more suitable variable for this?

    # NSIS doesn't always like forward slashes
    file(TO_NATIVE_PATH "${CPACK_PACKAGE_ICON}" CPACK_PACKAGE_ICON_NATIVE)
    set(CPACK_PACKAGE_ICON "${CPACK_PACKAGE_ICON_NATIVE}")
    file(TO_NATIVE_PATH "${CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP}" CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP_NATIVE)
    set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP_NATIVE}")

elseif(CPACK_GENERATOR STREQUAL "WIX")
    # for wix version values need to look like 'x.x.x.x' where x is an integer from 0 to 65534.
    set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
endif()
