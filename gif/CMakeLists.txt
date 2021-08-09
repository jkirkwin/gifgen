project(gif LANGUAGES CXX)

include_directories(include)

add_library(sub_block_buffer sub_block_buffer.cpp include/sub_block_buffer.hpp)

# Add unit test target
add_executable(test_sub_block_buffer test/test_sub_block_buffer.cpp)
target_link_libraries(test_sub_block_buffer Catch2::Catch2 sub_block_buffer)
ADD_COVERAGE_TARGET(test_sub_block_buffer)

# TODO add other utilities here?
