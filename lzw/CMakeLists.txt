project(lzw LANGUAGES CXX)

include_directories(include)

# Add unit tests for the header-only library
set(TEST_NAME test_${PROJECT_NAME})
add_executable(${TEST_NAME} test/test_lzw.cpp include/lzw.hpp)
target_link_libraries(${TEST_NAME} Catch2::Catch2)
ADD_COVERAGE_TARGET(${TEST_NAME})
