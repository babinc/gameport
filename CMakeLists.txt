cmake_minimum_required(VERSION 3.14)
project(gameport C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Common sources
set(COMMON_SRC
    src/main.c
    src/term.c
    src/ui.c
    src/catalog.c
    src/install.c
    src/util.c
)

# Per-game sources
file(GLOB GAME_SRC src/games/*.c)

# Platform layer
if(WIN32)
    set(PLATFORM_SRC src/platform_win32.c)
else()
    set(PLATFORM_SRC src/platform_posix.c)
endif()

add_executable(gameport ${COMMON_SRC} ${PLATFORM_SRC} ${GAME_SRC})

target_compile_options(gameport PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -pedantic>
    $<$<C_COMPILER_ID:MSVC>:/W4>
)

# Link math library on Unix
if(UNIX)
    target_link_libraries(gameport m)
endif()

# Windows-specific settings
if(WIN32)
    target_compile_definitions(gameport PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Install target
install(TARGETS gameport DESTINATION bin)
