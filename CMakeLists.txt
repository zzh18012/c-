cmake_minimum_required(VERSION 3.28)
project(NeonBulletArena LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.2
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(SFML)

add_executable(NeonBulletArena
    src/main.cpp
    src/core/Game.cpp
    src/entities/Player.cpp
    src/entities/Boss.cpp
    src/entities/BossManager.cpp
    src/entities/PlayerBullet.cpp
    src/entities/PlayerBulletCluster.cpp
    src/entities/PlayerBulletHoming.cpp
    src/entities/PlayerBulletOrbital.cpp
    src/entities/PlayerBulletPiercing.cpp
    src/entities/PlayerBulletSpread.cpp
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
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    sfml-main
)

# Copy SFML DLLs to output directory after build
add_custom_command(TARGET NeonBulletArena POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:sfml-graphics>"
        "$<TARGET_FILE_DIR:NeonBulletArena>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:sfml-window>"
        "$<TARGET_FILE_DIR:NeonBulletArena>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:sfml-system>"
        "$<TARGET_FILE_DIR:NeonBulletArena>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:sfml-audio>"
        "$<TARGET_FILE_DIR:NeonBulletArena>"
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
)
