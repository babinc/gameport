cmake_minimum_required(VERSION 3.14)
project(open-game-portal VERSION 0.4.1 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Common sources
set(COMMON_SRC
    src/main.c
    src/ui/term.c
    src/ui/ui.c
    src/core/catalog.c
    src/core/install.c
    src/core/util.c
)

# Per-game sources
file(GLOB GAME_SRC src/games/*.c)

# Platform layer
if(WIN32)
    set(PLATFORM_SRC src/core/platform_win32.c)
else()
    set(PLATFORM_SRC src/core/platform_posix.c)
endif()

add_executable(open-game-portal ${COMMON_SRC} ${PLATFORM_SRC} ${GAME_SRC})
target_compile_definitions(open-game-portal PRIVATE
    OGP_VERSION="${PROJECT_VERSION}"
)

target_compile_options(open-game-portal PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -pedantic>
    $<$<C_COMPILER_ID:MSVC>:/W4>
)

# Link math library on Unix
if(UNIX)
    target_link_libraries(open-game-portal m)
endif()

# Windows-specific settings
if(WIN32)
    target_compile_definitions(open-game-portal PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Install target
install(TARGETS open-game-portal DESTINATION bin)
