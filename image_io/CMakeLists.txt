project(image_io LANGUAGES CXX)

# Boost::GIL's IO extension has not yet been upgraded 
# to be C++20-compatible, so we have to use C++17 for 
# this library.
set(CMAKE_CXX_STANDARD 17) 

# The IO extension depends on libjpeg and libpng
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)

include_directories(include)

set(IMAGE_LIB_SOURCES image_io.cpp include/image_io.hpp)
add_library(${PROJECT_NAME} STATIC ${IMAGE_LIB_SOURCES})
target_link_libraries(${PROJECT_NAME} ${JPEG_LIBRARY} ${PNG_LIBRARY})
