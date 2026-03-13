cmake_minimum_required(VERSION 4.0)

project(game)

set(CMAKE_CXX_STANDARD 20)

add_executable(game
    src/main.cpp

    src/application.h
    src/application.cpp
    src/audio.h
    src/audio.cpp
    src/ui.h
    src/ui.cpp
    src/game.h
    src/game.cpp

    src/common.h
    src/common.cpp
    src/template.h

# we don't use imgui right now
#     vendor/imgui/imgui.cpp
#     vendor/imgui/imgui.h
#     vendor/imgui/imgui_draw.cpp
#     vendor/imgui/imgui_tables.cpp
#     vendor/imgui/imgui_widgets.cpp
#     vendor/imgui/imgui_demo.cpp
#
#     # the backends we use
#     vendor/imgui/backends/imgui_impl_sdl3.cpp
#     vendor/imgui/backends/imgui_impl_sdlrenderer3.cpp
#     vendor/imgui/backends/imgui_impl_sdl3.h
#     vendor/imgui/backends/imgui_impl_sdlrenderer3.h
)

target_include_directories(game PRIVATE vendor/imgui)
target_include_directories(game PRIVATE vendor/imgui/backends)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT game)

# SDL
add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)

# SDL_image
set(SDLIMAGE_VENDORED ON)
# turn off unused formats
set(SDLIMAGE_ANI  OFF)
set(SDLIMAGE_AVIF OFF)
set(SDLIMAGE_BMP  OFF)
set(SDLIMAGE_GIF  OFF)
set(SDLIMAGE_JPG  OFF)
set(SDLIMAGE_JXL  OFF)
set(SDLIMAGE_LBM  OFF)
set(SDLIMAGE_PCX  OFF)
set(SDLIMAGE_PNM  OFF)
set(SDLIMAGE_QOI  OFF)
set(SDLIMAGE_SVG  OFF)
set(SDLIMAGE_TGA  OFF)
set(SDLIMAGE_TIF  OFF)
set(SDLIMAGE_WEBP OFF)
set(SDLIMAGE_XCF  OFF)
set(SDLIMAGE_XPM  OFF)
set(SDLIMAGE_XV   OFF)
# png
set(SDLIMAGE_PNG ON)

add_subdirectory(vendor/SDL_image EXCLUDE_FROM_ALL)

# SDL_ttf
set(SDLTTF_VENDORED ON)
add_subdirectory(vendor/SDL_ttf EXCLUDE_FROM_ALL)

target_include_directories(game PRIVATE vendor/SDL/include/)
target_include_directories(game PRIVATE vendor/SDL_image/include/)
target_include_directories(game PRIVATE vendor/SDL_ttf/include/)

target_link_libraries(
    game
    PRIVATE
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
)
