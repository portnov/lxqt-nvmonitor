cmake_minimum_required(VERSION 3.16)
project(nvmonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Use architecture-specific lib dir on Debian/Ubuntu (e.g. lib/x86_64-linux-gnu)
include(GNUInstallDirs)

# Path to installed lxqt libraries (override with -DLXQT_INSTALL_PREFIX=/path/to/lxqt)
set(LXQT_INSTALL_PREFIX "" CACHE PATH "Path to installed lxqt libraries (empty = system default)")
if(NOT LXQT_INSTALL_PREFIX STREQUAL "")
    set(CMAKE_PREFIX_PATH "${LXQT_INSTALL_PREFIX}" ${CMAKE_PREFIX_PATH})
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(lxqt REQUIRED)
find_package(KF6WindowSystem REQUIRED)

set(HEADERS
    nvmonitorplugin.h
    nvmonitorconfiguration.h
)

set(SOURCES
    nvmonitorplugin.cpp
    nvmonitor.cpp
    nvmonitorconfiguration.cpp
)

set(UIS
    nvmonitorconfiguration.ui
)

# Shared library (plugin)
add_library(nvmonitor MODULE ${HEADERS} ${SOURCES} ${UIS})

# lxqt-panel installs only a subset of headers system-wide.
# Try the system path first; if lxqtpanelpluginconfigdialog.h is missing,
# fall back to the source tree (override with -DLXQT_PANEL_SRC_DIR=/path/to/lxqt-panel).
set(LXQT_PANEL_SRC_DIR "${CMAKE_SOURCE_DIR}/../lxqt-panel" CACHE PATH "Path to lxqt-panel source tree")

find_path(LXQT_PANEL_HEADER_DIR lxqtpanelpluginconfigdialog.h
    PATHS /usr/include/lxqt
    NO_DEFAULT_PATH)
if(LXQT_PANEL_HEADER_DIR)
    set(LXQT_PANEL_INCLUDE_DIR "${LXQT_PANEL_HEADER_DIR}")
else()
    set(LXQT_PANEL_INCLUDE_DIR "${LXQT_PANEL_SRC_DIR}/panel")
    if(NOT EXISTS "${LXQT_PANEL_INCLUDE_DIR}/lxqtpanelpluginconfigdialog.h")
        message(FATAL_ERROR "lxqtpanelpluginconfigdialog.h not found. Set -DLXQT_PANEL_SRC_DIR=/path/to/lxqt-panel")
    endif()
endif()

target_link_libraries(nvmonitor
    Qt6::Widgets
    lxqt
    KF6::WindowSystem
)

target_include_directories(nvmonitor PRIVATE "${LXQT_PANEL_INCLUDE_DIR}")

set_target_properties(nvmonitor PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Generate .desktop file from .desktop.in
configure_file(resources/nvmonitor.desktop.in nvmonitor.desktop @ONLY)

# Installation
install(TARGETS nvmonitor DESTINATION "${CMAKE_INSTALL_LIBDIR}/lxqt-panel")
install(FILES "${CMAKE_BINARY_DIR}/nvmonitor.desktop" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/lxqt/lxqt-panel")

# Tests (optional, requires Qt6Test)
option(BUILD_TESTS "Build tests for the plugin" ON)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
