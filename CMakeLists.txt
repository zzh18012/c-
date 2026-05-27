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
    src/entities/PlayerBullet.cpp
    src/entities/BossBullet.cpp
    src/entities/Item.cpp
    src/systems/CollisionSystem.cpp
    src/systems/ParticleSystem.cpp
    src/systems/CameraShake.cpp
    src/systems/BackgroundSystem.cpp
    src/ui/HUD.cpp
    src/ui/Button.cpp
    src/ui/Menu.cpp
)

target_include_directories(NeonBulletArena PRIVATE src)
target_link_libraries(NeonBulletArena PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
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
)
