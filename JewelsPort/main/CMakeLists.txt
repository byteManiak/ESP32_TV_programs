FILE(GLOB_RECURSE app_sources (*.*))

set(include_dirs ".")

idf_component_register(SRCS main.cpp game/bar.cpp game/board.cpp game/game.cpp game/gem.cpp game/mainmenu.cpp game/score.cpp engine/sprite.cpp engine/texture.cpp engine/color.cpp
                    INCLUDE_DIRS ${include_dirs})
