cmake_minimum_required(VERSION 3.16)

project(fists-of-fury VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options()
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wcast-align -ggdb -O0 -Werror=switch)
endif()

add_subdirectory(vendor/SDL)
add_subdirectory(vendor/SDL_image)
add_subdirectory(vendor/SDL_ttf)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/draw.cpp
    src/game.cpp
    src/vec2.cpp
    src/sprite.cpp
    src/settings.cpp
    src/debug_menu.cpp
    src/animation.cpp
    src/level_info.cpp
    src/entities/enemy.cpp
    src/entities/knife.cpp
    src/entities/entity.cpp
    src/entities/barrel.cpp
    src/entities/player.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
)

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3_image::SDL3_image>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3_ttf::SDL3_ttf>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()
