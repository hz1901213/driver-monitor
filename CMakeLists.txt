cmake_minimum_required(VERSION 3.15)
project(DriverMonitor VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find DirectX 11
find_library(D3D11_LIBRARY d3d11)
find_library(DXGI_LIBRARY dxgi)
find_library(D3DCOMPILER_LIBRARY d3dcompiler)

if(NOT D3D11_LIBRARY)
    message(FATAL_ERROR "DirectX 11 not found. Please install Windows SDK.")
endif()

# ImGui source files
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/libs/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

# Core source files
set(CORE_SOURCES
    src/core/Utils.cpp
    src/core/Config.cpp
    src/core/EventManager.cpp
    src/core/DriverMonitor.cpp
)

# Monitoring source files
set(MONITORING_SOURCES
    src/monitoring/ETWConsumer.cpp
    src/monitoring/RegistryMonitor.cpp
    src/monitoring/FileSystemMonitor.cpp
    src/monitoring/WMIMonitor.cpp
)

# GUI source files
set(GUI_SOURCES
    src/gui/MainWindow.cpp
    src/gui/EventLogPanel.cpp
    src/gui/DetailsPanel.cpp
    src/gui/SettingsPanel.cpp
    src/gui/StatisticsPanel.cpp
)

# Application source files
set(APP_SOURCES
    src/Application.cpp
    src/main.cpp
)

# All sources
set(ALL_SOURCES
    ${IMGUI_SOURCES}
    ${CORE_SOURCES}
    ${MONITORING_SOURCES}
    ${GUI_SOURCES}
    ${APP_SOURCES}
)

# Create executable
add_executable(DriverMonitor WIN32 ${ALL_SOURCES})

# Include directories
target_include_directories(DriverMonitor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Link libraries
target_link_libraries(DriverMonitor PRIVATE
    ${D3D11_LIBRARY}
    ${DXGI_LIBRARY}
    ${D3DCOMPILER_LIBRARY}
    d3d11.lib
    dxgi.lib
    d3dcompiler.lib
    advapi32.lib    # Registry
    wbemuuid.lib    # WMI
    tdh.lib         # ETW
    ntdll.lib       # NT APIs
    version.lib     # File version info
    wintrust.lib    # Digital signatures
    crypt32.lib     # Crypto
    comctl32.lib    # Common controls
)

# Set subsystem to Windows (not console)
set_target_properties(DriverMonitor PROPERTIES
    WIN32_EXECUTABLE TRUE
    LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
)

# Add compile definitions
target_compile_definitions(DriverMonitor PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
)

# Set warnings
if(MSVC)
    target_compile_options(DriverMonitor PRIVATE /W4)
else()
    target_compile_options(DriverMonitor PRIVATE -Wall -Wextra -pedantic)
endif()

# Copy config.json to output directory
add_custom_command(TARGET DriverMonitor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/config.json
    $<TARGET_FILE_DIR:DriverMonitor>/config.json
)
