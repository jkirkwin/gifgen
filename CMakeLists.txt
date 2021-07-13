cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

project (cpp_project LANGUAGES CXX)

# Cause the build to fail if C++20 is not available. 
# Subprojects using the IO extension for Boost::GIL 
# Will set this to a different version.
set(CXX_STANDARD ON) 
set(CMAKE_CXX_STANDARD 20)

# Enable all the extra warnings we can
add_compile_options(-Wall -Wextra -Werror -pedantic)

# Ensure this remains *after* the project command or it will be clobbered.
set(CMAKE_VERBOSE_MAKEFILE true)

# Imports and includes
find_package(Boost REQUIRED)
# find_package(Catch2 REQUIRED) # TODO Add catch support

# Add image IO subproject built with C++17
add_subdirectory(image_io)

# Add other C++20 sub-modules
add_subdirectory(args)

# Add main application to demo PNG/JPEG input
add_executable(main main.cpp)
target_include_directories(main PRIVATE image_io/include args/include)
target_link_libraries(main image_io args)

# TODO Add coverage and sanitization stuff
