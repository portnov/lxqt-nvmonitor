# Тесты для плагина nvmonitor

find_package(Qt6Test REQUIRED)

# Макрос для создания тестовых исполняемых файлов
macro(add_test_executable _executable)
    add_executable(${_executable} ${ARGN})
    add_test(${_executable} ${_executable})
endmacro(add_test_executable)

# ============================================================================
# Тест 1: Тестирование NVML (загрузка библиотеки и получение данных GPU)
# ============================================================================
set(nvml_test_SRCS
    test_nvml.cpp
)

add_test_executable(test_nvml ${nvml_test_SRCS})
target_link_libraries(test_nvml
    Qt6::Test
    Qt6::Widgets
    Qt6::Core
    dl
)

# ============================================================================
# Тест 2: Тестирование настроек (сохранение/загрузка)
# ============================================================================
set(settings_test_SRCS
    test_settings.cpp
)

add_test_executable(test_settings ${settings_test_SRCS})
target_link_libraries(test_settings
    Qt6::Test
    Qt6::Widgets
    Qt6::Core
)

# ============================================================================
# Тест 3: Тестирование виджета графика
# ============================================================================
set(widget_test_SRCS
    test_widget.cpp
)

add_test_executable(test_widget ${widget_test_SRCS})
target_link_libraries(test_widget
    Qt6::Test
    Qt6::Widgets
    Qt6::Core
)

# Включаем тестирование
enable_testing()
