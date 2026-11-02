cmake_minimum_required(VERSION 3.20)

project(dolphin-colortags
    VERSION 1.0.0
    DESCRIPTION "Color tags for files and folders in Dolphin"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOMOC ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra)
endif()

include(GNUInstallDirs)
include(CTest)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets DBus)
find_package(KF6KIO REQUIRED COMPONENTS KIOCore KIOWidgets)
find_package(KF6CoreAddons REQUIRED)

# Qt's plugin directory, relative to the install prefix. Distros with a
# multiarch libdir (e.g. Debian) are handled by CMAKE_INSTALL_LIBDIR.
set(COLORTAG_PLUGIN_DIR "${CMAKE_INSTALL_LIBDIR}/qt6/plugins" CACHE PATH
    "Directory where Qt plugins are installed (relative to the prefix)")

add_library(colortagcore STATIC
    src/colortag.cpp
    src/colortag.h
)
target_include_directories(colortagcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(colortagcore PUBLIC Qt6::Core Qt6::Gui PRIVATE Qt6::DBus)

add_library(colortagui STATIC
    src/colorpicker.cpp
    src/colorpicker.h
    src/icontools.cpp
    src/icontools.h
)
target_link_libraries(colortagui PUBLIC colortagcore Qt6::Gui Qt6::Widgets)

# ---------------------------------------------------------------------------
# Dolphin overlay plugin: paints the colored badge on icons
# ---------------------------------------------------------------------------
qt_add_plugin(colortagoverlay CLASS_NAME ColorTagOverlay src/colortagoverlay.cpp)
target_link_libraries(colortagoverlay PRIVATE colortagcore KF6::KIOCore)
set_target_properties(colortagoverlay PROPERTIES
    PREFIX ""
    OUTPUT_NAME "colortagoverlay"
    # Dev convenience: a ready-to-use QT_PLUGIN_PATH layout in the build dir.
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/kf6/overlayicon"
)

# ---------------------------------------------------------------------------
# Dolphin context menu plugin: "Color Tag" submenu + picker
# ---------------------------------------------------------------------------
qt_add_plugin(colortagaction CLASS_NAME ColorTagAction src/colortagaction.cpp)
target_link_libraries(colortagaction PRIVATE
    colortagui
    KF6::KIOWidgets
    KF6::KIOCore
    KF6::CoreAddons
)
set_target_properties(colortagaction PROPERTIES
    PREFIX ""
    OUTPUT_NAME "colortagaction"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/kf6/kfileitemaction"
)

# ---------------------------------------------------------------------------
# Command line helper
# ---------------------------------------------------------------------------
add_executable(colortag src/main.cpp)
target_link_libraries(colortag PRIVATE colortagui)
target_compile_definitions(colortag PRIVATE COLORTAG_VERSION="${PROJECT_VERSION}")

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(BUILD_TESTING)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Test)
    add_executable(test_colortag tests/test_colortag.cpp)
    target_link_libraries(test_colortag PRIVATE colortagcore Qt6::Test)
    add_test(NAME colortag COMMAND test_colortag)
endif()

# ---------------------------------------------------------------------------
# Installation
# ---------------------------------------------------------------------------
install(TARGETS colortagoverlay
    LIBRARY DESTINATION "${COLORTAG_PLUGIN_DIR}/kf6/overlayicon"
)
install(TARGETS colortagaction
    LIBRARY DESTINATION "${COLORTAG_PLUGIN_DIR}/kf6/kfileitemaction"
)
install(TARGETS colortag
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
install(DIRECTORY icons/
    DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/emblems"
    FILES_MATCHING PATTERN "colortag*.svg"
)
install(FILES README.md LICENSE
    DESTINATION "${CMAKE_INSTALL_DATADIR}/doc/dolphin-colortags"
)

message(STATUS "Dolphin Color Tags ${PROJECT_VERSION}")
message(STATUS "  plugins: ${CMAKE_INSTALL_PREFIX}/${COLORTAG_PLUGIN_DIR}")
message(STATUS "  binary:  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/colortag")
