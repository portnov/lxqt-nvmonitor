# Tests for the nvmonitor plugin

find_package(Qt6Test REQUIRED)

# Macro to create test executables
macro(add_test_executable _executable)
    add_executable(${_executable} ${ARGN})
    add_test(${_executable} ${_executable})
endmacro(add_test_executable)

# ============================================================================
# Test 1: NVML library loading and GPU data retrieval
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
# Test 2: Settings save/load
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
# Test 3: Graph widget rendering
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

# Enable testing
enable_testing()
