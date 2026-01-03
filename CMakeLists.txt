cmake_minimum_required(VERSION 3.16)
project(multitail C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/main.c
    src/console.c
    src/pane.c
    src/linebuf.c
    src/input.c
    src/statusbar.c
)

# Create executable
add_executable(multitail ${SOURCES})

# Windows-specific settings
if(MSVC)
    # Use static runtime for easier distribution
    set_property(TARGET multitail PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    # Disable security warnings for standard C functions
    target_compile_definitions(multitail PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Console subsystem
if(WIN32)
    set_target_properties(multitail PROPERTIES
        WIN32_EXECUTABLE FALSE)
endif()
