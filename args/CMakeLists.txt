project(args LANGUAGES CXX)

include_directories(include)

set(ARGS_LIB_SOURCES args.cpp include/args.hpp)
add_library(${PROJECT_NAME} STATIC ${ARGS_LIB_SOURCES})

# TODO Add some tests for the args library. 
# Can do this either with a shell script, or (preferably) with Catch.
# If catch is used then need to refactor the interface to return a status
# value and modify the args struct as an argument
