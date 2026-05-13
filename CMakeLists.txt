cmake_minimum_required(VERSION 3.16)
project(nvmonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

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

# Динамическая библиотека (плагин)
add_library(nvmonitor MODULE ${HEADERS} ${SOURCES} ${UIS})

# Добавляем include-пути для заголовков lxqt-panel
# (они не установлены в систему, доступны только в дереве исходников)
set(LXQT_PANEL_SRC_DIR "${CMAKE_SOURCE_DIR}/../lxqt-panel")

# Проверяем, установлены ли заголовки lxqt-panel в систему
find_path(LXQT_PANEL_INCLUDE_DIR
    NAMES ilxqtpanelplugin.h
    PATHS /usr/include /usr/local/include /home/portnov/soft/lxqt/include
    NO_DEFAULT_PATH
)

if(NOT LXQT_PANEL_INCLUDE_DIR)
    # Используем заголовки из дерева исходников lxqt-panel
    set(LXQT_PANEL_INCLUDE_DIR "${LXQT_PANEL_SRC_DIR}/panel")
endif()

include_directories(${LXQT_PANEL_INCLUDE_DIR})

target_link_libraries(nvmonitor
    Qt6::Widgets
    lxqt
    KF6::WindowSystem
)

set_target_properties(nvmonitor PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    PREFIX ""  # без префикса lib
)

# Генерируем .desktop файл из .desktop.in
configure_file(resources/nvmonitor.desktop.in nvmonitor.desktop @ONLY)

# Установка
install(TARGETS nvmonitor DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/lxqt-panel/")
install(FILES "${CMAKE_BINARY_DIR}/nvmonitor.desktop" DESTINATION "${CMAKE_INSTALL_PREFIX}/share/lxqt/lxqt-panel/")
