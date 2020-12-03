# Copyright - 2016-2020 - Jan Christoph Uhde <Jan@UhdeJC.com>

cmake_minimum_required(VERSION 3.8)
project(ext-math VERSION 0.0.1 LANGUAGES CXX)
message(STATUS "extINFO -- entering ext-math")

## OPTIONS
option(EXTMATH_WARNINGS "enable warnings" ON)
option(EXTMATH_CHECKED "user assert" ON)
option(EXTMATH_TESTS "build tests" OFF)

# enable extcpp cmake
include(${CMAKE_CURRENT_LIST_DIR}/ext_cmake_enable.cmake)
include(ext_cmake_setup)
ext_enable_ext_libs(basics)

find_package(Threads REQUIRED)

if(LINUX)
    set(EXT_OUTDIR "")
elseif(MSVC)
    #TODO - move settings below into corresponding targets
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${EXT_OUTDIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${EXT_OUTDIR}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${EXT_OUTDIR}")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
endif()

# include build dir to find version.hpp
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# required by folder structure for XCode and VisualStudio (includes)
# sources are always required
include(src_files.cmake)
include(include_files.cmake)

add_library(ext-math INTERFACE ${ext-math-source} ${ext-math-header})
target_include_directories(ext-math INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
)

target_link_libraries(ext-math INTERFACE
    ext::basics
)

# set up folder structure for XCode and VisualStudio
#set_target_properties (ext-math PROPERTIES FOLDER math)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ext-math-header} ${ext-math-source})

add_library(ext::math ALIAS ext-math)

## testing
if(EXTMATH_TESTS)
    ext_log("ext-math tests enabled")
    include(CTest)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    ext_add_test_subdirectory("google" tests)
else()
    ext_log("ext-math tests disabled")
endif()

## installation
if(COMMAND ext_install)
    set_target_properties(ext-math PROPERTIES EXPORT_NAME math)

    ext_install(ext-math include/ext)

    install(TARGETS ext-math
        EXPORT ext-math-targets
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
	install(
        EXPORT ext-math-targets
		FILE ext-math-config.cmake
        NAMESPACE ext::
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake
    )
endif()

ext_log("generating version information")
add_custom_target(
    ext_math_version_update ALL
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND ${CMAKE_COMMAND}
                -D "EXT_GIT_VERSION_OUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/ext_math_version.hpp"
                -P "${ext_cmake_dir}/ext_script_git_version.cmake"
)
