project(gif LANGUAGES CXX)

include_directories(include)

# Targets for the sub-block buffer library
set(BUFFER_LIBRARY gif_block_buffer)
set(BUFFER_TEST test_${BUFFER_LIBRARY})
add_library(${BUFFER_LIBRARY} ${BUFFER_LIBRARY}.cpp include/${BUFFER_LIBRARY}.hpp)
add_executable(${BUFFER_TEST} test/${BUFFER_TEST}.cpp)
target_link_libraries(${BUFFER_TEST} Catch2::Catch2 ${BUFFER_LIBRARY})
ADD_COVERAGE_TARGET(${BUFFER_TEST})

# Targets for the GIF builder library
add_library(gif_builder gif_builder.cpp include/gif_builder.hpp)
target_link_libraries(gif_builder palettize ${BUFFER_LIBRARY})
