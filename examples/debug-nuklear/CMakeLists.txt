CMAKE_MINIMUM_REQUIRED(VERSION 3.16...3.24 FATAL_ERROR)

PROJECT(peanutgb-debugger)
ADD_EXECUTABLE(peanutgb-debugger src/main.c src/nuklear.c src/overview.c ../../peanut_gb.h)
TARGET_INCLUDE_DIRECTORIES(peanutgb-debugger PRIVATE inc)

set_property(TARGET peanutgb-debugger PROPERTY VS_DPI_AWARE "PerMonitor")

#sdl2
INCLUDE(CPM.cmake)
CPMADDPACKAGE(GITHUB_REPOSITORY libsdl-org/SDL
        NAME SDL2
        GIT_TAG release-2.26.1
        OPTIONS
        "SDL_SHARED_ENABLED_BY_DEFAULT OFF"
        "SDL_STATIC_ENABLED_BY_DEFAULT ON")
ADD_COMPILE_DEFINITIONS(SDL_MAIN_HANDLED SDL_LEAN_AND_MEAN)
TARGET_LINK_LIBRARIES(peanutgb-debugger PRIVATE SDL2-static)
