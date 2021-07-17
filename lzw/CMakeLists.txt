project(lzw LANGUAGES CXX)

include_directories(include)

set(LZW_LIB_SOURCES lzw.cpp include/lzw.hpp)
add_library(${PROJECT_NAME} STATIC ${LZW_LIB_SOURCES})

set(TEST_NAME test_${PROJECT_NAME})
add_executable(${TEST_NAME} test/test_lzw.cpp ${LZW_LIB_SOURCES})
target_link_libraries(${TEST_NAME} Catch2::Catch2)
ADD_COVERAGE_TARGET(${TEST_NAME})
