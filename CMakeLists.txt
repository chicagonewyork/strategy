cmake_minimum_required(VERSION 3.26)

find_package(Git QUIET)

# update code
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Updating Git submodules")

        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "Failed to update Git submodules")
        endif()        

        execute_process(COMMAND ${GIT_EXECUTABLE} submodule foreach  git pull origin main
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "Failed to pull Git submodules")
        endif()
                
    endif()
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(${CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} -g")
else()
    set(${CMAKE_CXX_FLAGS} "${CMAKE_CXX_FLAGS} -O3 -mfmaf")
endif()

# we are using g++ as the compiler
set(CMAKE_CXX_COMPILER g++)
# we are using native architecture
set(ARCH native)
# we are using c++ 20
set(CMAKE_CXX_STANDARD 20)

# LIBS_PATH is used to speficy path to libraries 
set(LIBS_PATH ${CMAKE_SOURCE_DIR}/core/cpp/src/lib)
# LIBS_PATH is used to speficy path to external
set(EXTERNAL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/core/cpp/external)

# google test variables
set(GTEST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/core/cpp/external/googletest)
set(GTest_DIR ${CMAKE_SOURCE_DIR}/core/cpp/external/googletest)
set(GTEST_INCLUDE_DIR ${GTEST_SOURCE_DIR}/googletest/include)

# path to json parser
set(JSON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/nlohmann/single_include/nlohmann)

# compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=${ARCH} -Wall -Werror -Wformat -Wreturn-type -Winit-self -Wimplicit-fallthrough -fno-rtti -fstrict-enums -Wdeprecated-copy -Wreorder -Wold-style-cast -Wextra-semi -Wsized-deallocation -rdynamic -Wsuggest-override -I${GTEST_INCLUDE_DIR} -I/usr/include")

project(demo_strategy CXX)
add_executable(demo_strategy demo_strategy.cpp)

# this line enable testing
enable_testing()
# this lines includes google test module
include(GoogleTest)

# this line links the libraries to the project
target_link_libraries(demo_strategy basics config_tree books control networking order_entry market_data order_gateway strategy_framework)
# this line includes the headers of the libraries
target_include_directories(demo_strategy PRIVATE ${PROJECT_SOURCE_DIR}/core/cpp/src/lib)

# this macro is used to get all the subdirectories in a directory
MACRO(SUBDIRLIST result curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(dirlist "")
    FOREACH(child ${children})
        IF(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        ENDIF()
    ENDFOREACH()
    SET(${result} ${dirlist})
ENDMACRO()

# scan the external directory for all the subdirectories
SUBDIRLIST(EXTERNAL_SUBDIRS ${EXTERNAL_PATH})

# this loop includes all the external libraries
FOREACH(external_dir ${EXTERNAL_SUBDIRS})
    add_subdirectory(${EXTERNAL_PATH}/${external_dir})
    include_directories(${EXTERNAL_PATH}/${external_dir})
ENDFOREACH()

# this is required for all projects to incude the libraries
include_directories(${LIBS_PATH})
# this line includes all the libraries
add_subdirectory(${LIBS_PATH})
