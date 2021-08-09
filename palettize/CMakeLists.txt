project(palettize LANGUAGES CXX)

include_directories(include)

add_library(color_table color_table.cpp include/color_table.hpp)
add_library(palettize palettize.cpp include/palettize.hpp)
target_link_libraries(palettize color_table)

# TODO Clean this up and add tests

