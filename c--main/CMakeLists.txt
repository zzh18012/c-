cmake_minimum_required(VERSION 3.28)
project(NeonBulletArena LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable LTO explicitly to work around link issues
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)

find_package(SFML REQUIRED COMPONENTS Graphics Window System Audio)

add_executable(NeonBulletArena
    src/main.cpp
    src/core/Game.cpp
    src/entities/Player.cpp
    src/entities/Boss.cpp
    src/entities/PlayerBullet.cpp
    src/entities/BossBullet.cpp
    src/entities/Item.cpp
    src/systems/CollisionSystem.cpp
    src/systems/ParticleSystem.cpp
    src/systems/CameraShake.cpp
    src/systems/BackgroundSystem.cpp
    src/systems/AudioSystem.cpp
    src/ui/HUD.cpp
    src/ui/Button.cpp
    src/ui/Menu.cpp
)

target_include_directories(NeonBulletArena PRIVATE src)
target_link_libraries(NeonBulletArena PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
)

# Copy SFML DLLs to output directory after build
add_custom_command(TARGET NeonBulletArena POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        C:/msys64/ucrt64/bin/libsfml-graphics-3.dll
        $<TARGET_FILE_DIR:NeonBulletArena>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        C:/msys64/ucrt64/bin/libsfml-window-3.dll
        $<TARGET_FILE_DIR:NeonBulletArena>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        C:/msys64/ucrt64/bin/libsfml-system-3.dll
        $<TARGET_FILE_DIR:NeonBulletArena>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        C:/msys64/ucrt64/bin/libsfml-audio-3.dll
        $<TARGET_FILE_DIR:NeonBulletArena>
)

# Copy MinGW runtime DLLs for standalone execution
get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
add_custom_command(TARGET NeonBulletArena POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
        "$<TARGET_FILE_DIR:NeonBulletArena>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MINGW_BIN_DIR}/libstdc++-6.dll"
        "$<TARGET_FILE_DIR:NeonBulletArena>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MINGW_BIN_DIR}/libwinpthread-1.dll"
        "$<TARGET_FILE_DIR:NeonBulletArena>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        C:/msys64/ucrt64/bin/libopenal-1.dll
        "$<TARGET_FILE_DIR:NeonBulletArena>"
)