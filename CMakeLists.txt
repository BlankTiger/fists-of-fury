cmake_minimum_required(VERSION 3.16)

project(fists-of-fury VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wcast-align -ggdb -Werror=switch)
endif()

add_subdirectory(vendor/SDL)
add_subdirectory(vendor/SDL_image)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/draw.cpp
    src/sprite.cpp
    src/level_info.cpp
    src/entities/enemy.cpp
    src/entities/entity.cpp
    src/entities/barrel.cpp
    src/entities/player.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    SDL3_image::SDL3_image
    SDL3::SDL3
)

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()
