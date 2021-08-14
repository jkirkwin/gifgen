project(args LANGUAGES CXX)

include_directories(include)

set(ARGS_LIB_SOURCES args.cpp include/args.hpp)
add_library(${PROJECT_NAME} STATIC ${ARGS_LIB_SOURCES})
