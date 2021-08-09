project(gif LANGUAGES CXX)

# TODO Cannot find the palettize header (or assumeably lzw either)

include_directories(include)

# Targets for the sub-block buffer library
add_library(sub_block_buffer sub_block_buffer.cpp include/sub_block_buffer.hpp)
add_executable(test_sub_block_buffer test/test_sub_block_buffer.cpp)
target_link_libraries(test_sub_block_buffer Catch2::Catch2 sub_block_buffer)
ADD_COVERAGE_TARGET(test_sub_block_buffer)

# Targets for the GIF builder library
add_library(gif_builder gif_builder.cpp include/gif_builder.hpp)
target_link_libraries(gif_builder palettize sub_block_buffer)

# TODO Unit tests for the GIF builder?
