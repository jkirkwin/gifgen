project(palettize LANGUAGES CXX)

include_directories(include)

# Color table
add_library(color_table color_table.cpp include/color_table.hpp)

# TODO Add tests for color table

# Median cut
add_library(median_cut median_cut.cpp include/median_cut.hpp)
target_link_libraries(median_cut color_table)

add_executable(test_median_cut test/test_median_cut.cpp)
target_link_libraries(test_median_cut median_cut)
ADD_COVERAGE_TARGET(test_median_cut)

# Main palettization library for client use
add_library(palettize palettize.cpp include/palettize.hpp)
target_link_libraries(palettize median_cut)
