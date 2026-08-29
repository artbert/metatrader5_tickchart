cmake_minimum_required(VERSION 3.20)
project(MT5TickChartDLL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use MinGW-specific flags
if (MINGW)
    add_compile_options(
        -O3
        -msse2
        -ffast-math
        -fno-exceptions
        -fno-rtti
    )
endif()

add_compile_options(-fexceptions)

# DLL target
add_library(MT5TickChartDLL SHARED
    src/main.cpp
    src/mt5_tick_chart_lib.cpp
    src/chart_module.cpp
    src/tick_chart.cpp
    src/bar_chart.cpp
    src/canvas.cpp
    src/chart_canvas.cpp
    src/resource.cpp
    src/mt5_tick_chart_lib.def
    src/mt5_tick_chart.rc
    src/small.ico
    src/mt5_tick_chart.ico
)

target_include_directories(MT5TickChartDLL PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Export definitions
target_compile_definitions(MT5TickChartDLL PRIVATE
    WIN32
    NDEBUG
    _WINDOWS
    _USRDLL
    MT5TICKCHARTDLL_EXPORTS
    UNICODE _UNICODE
)

# Link Windows libraries
target_link_libraries(MT5TickChartDLL PRIVATE
    user32
    gdi32
    gdiplus
    kernel32
    winmm
    msimg32
)

set_target_properties(MT5TickChartDLL PROPERTIES 
    PREFIX ""
)