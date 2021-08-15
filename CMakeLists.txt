cmake_minimum_required (VERSION 3.20 FATAL_ERROR)

project (cpp_project LANGUAGES CXX)

# Cause the build to fail if C++20 is not available. 
# Subprojects using the IO extension for Boost::GIL 
# will set this to a different version.
set(CXX_STANDARD ON) 
set(CMAKE_CXX_STANDARD 20)

# Enable all the extra warnings we can
add_compile_options(-Wall -Wextra -Werror -pedantic)

# Ensure this remains *after* the project command or it will be clobbered.
set(CMAKE_VERBOSE_MAKEFILE true)

# Imports and includes
find_package(Boost REQUIRED)
find_package(Catch2 REQUIRED)
include_directories(common)

# We add this here because LZW is a header only library, and this is 
# the easiest way to allow other libraries to depend on it without
# nasty relative paths.
include_directories(lzw/include)

# Coverage option and shared coverage setup logic
option(ENABLE_COVERAGE "Enable code coverage with Lcov" false)

if(ENABLE_COVERAGE)
    # Ensure build type is debug before continuing.
    message(STATUS "Forcing CMAKE_BUILD_TYPE to DEBUG")
    set(CMAKE_BUILD_TYPE Debug)

    # Include the main lcov machinery and add the necessary compiler flags
    include(CodeCoverage.cmake)
    append_coverage_compiler_flags()
endif()

function(ADD_COVERAGE_TARGET TEST_TARGET)
    if (ENABLE_COVERAGE)
        set(COVERAGE_TARGET_NAME ${TEST_TARGET}_coverage)
        message(STATUS "Adding coverage target ${COVERAGE_TARGET_NAME}")

        setup_target_for_coverage_lcov(
            NAME ${COVERAGE_TARGET_NAME}
            EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/${TEST_TARGET} 
            LCOV_ARGS --rc lcov_branch_coverage=1
            GENHTML_ARGS --legend --branch-coverage
            DEPENDENCIES ${TEST_TARGET}
        )
    else()
        message(STATUS "Skipping coverage target ${TEST_TARGET} (ENABLE_COVERAGE is OFF)")
    endif()
endfunction()

# Add image IO subproject built with C++17
add_subdirectory(image_io)

# Add other C++20 sub-modules
add_subdirectory(args)
add_subdirectory(lzw)
add_subdirectory(palettize)
add_subdirectory(gif)

# Targets for the application
set (APP_NAME gifgen)
add_executable(${APP_NAME} gifgen.cpp)

set (APP_INCLUDE_DIRS args/include image_io/include gif/include)
target_include_directories(${APP_NAME} PRIVATE ${APP_INCLUDE_DIRS})

set (APP_LINK_LIBS image_io args gif_builder)
target_link_libraries(${APP_NAME} ${APP_LINK_LIBS})

# Add the gifgen application and demo script to the install
# bin directory. Note that a release build should be used for 
# installing to prevent the application from running incredibly
# slowly on very large inputs.
install(TARGETS ${APP_NAME} DESTINATION bin)
install(
    FILES demo 
    DESTINATION bin
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                GROUP_READ GROUP_EXECUTE 
                WORLD_EXECUTE
)
install(DIRECTORY sample_images DESTINATION bin)

# Add a target for the degif debugging tool.
add_executable(degif degif.cpp)
