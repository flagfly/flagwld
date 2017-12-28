cmake_minimum_required(VERSION 2.6)

project(flagwld C CXX)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CXX_FLAGS
 -g
 # -DVALGRIND
 -DFLAGWLD_STD_STRING
 -DCHECK_PTHREAD_RETURN_VALUE
 -D_FILE_OFFSET_BITS=64
 -Wall
 -Wextra
 #-Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=nocona
 # -MMD
 # -std=c++0x
 -rdynamic
 )
if(CMAKE_BUILD_BITS EQUAL 32)
  list(APPEND CXX_FLAGS "-m32")
endif()
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "g++")
#set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -pg")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

#http://www.cnblogs.com/vv1133/articles/2633669.html
#http://en.wikipedia.org/wiki/Aliasing_%28computing%29
#set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -fno-strict-aliasing -DNDEBUG")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

if(NOT DEFINED ENV{CONFIG_TIMESTAMP})
  add_definitions(-DPACKAGE_TIMESTAMP=\"null\")
else()
  add_definitions(-DPACKAGE_TIMESTAMP=\"$ENV{CONFIG_TIMESTAMP}\")
endif()

find_package(Boost REQUIRED)
#find_package(Protobuf)
find_package(CURL)
find_package(ZLIB)
find_path(CARES_INCLUDE_DIR ares.h)
find_library(CARES_LIBRARY NAMES cares)
find_path(MHD_INCLUDE_DIR microhttpd.h)
find_library(MHD_LIBRARY NAMES microhttpd)
find_library(BOOSTTEST_LIBRARY NAMES boost_unit_test_framework)
find_library(BOOSTPO_LIBRARY NAMES boost_program_options)
find_path(TCMALLOC_INCLUDE_DIR google/heap-profiler.h)
find_library(TCMALLOC_LIBRARY NAMES tcmalloc_and_profiler)
find_path(HIREDIS_INCLUDE_DIR hiredis/hiredis.h)
find_library(HIREDIS_LIBRARY NAMES hiredis)
find_path(GD_INCLUDE_DIR gd.h)
find_library(GD_LIBRARY NAMES gd)


if(CARES_INCLUDE_DIR AND CARES_LIBRARY)
  message(STATUS "found cares")
endif()
if(PROTOBUF_FOUND)
  message(STATUS "found protobuf")
endif()
if(CURL_FOUND)
  message(STATUS "found curl")
endif()
if(TCMALLOC_INCLUDE_DIR AND TCMALLOC_LIBRARY)
  message(STATUS "found tcmalloc")
else()
  message(STATUS "not found tcmalloc")
endif()
if(ZLIB_FOUND)
  message(STATUS "found zlib")
endif()

if(HIREDIS_INCLUDE_DIR AND HIREDIS_LIBRARY)
  message(STATUS "found hiredis")
endif()

if(GD_INCLUDE_DIR AND GD_LIBRARY)
  message(STATUS "found gd")
endif()

include_directories( /opt/libev/include/ )
link_directories ( /opt/libev/lib/ ) 

link_directories ( /usr/lib64/ ) 
link_directories ( /usr/lib64/mysql/ ) 

include_directories( /opt/libz/include )
link_directories ( /opt/libz/lib )

include_directories( /opt/libjio/include )
link_directories ( /opt/libjio/lib )

#include_directories( /opt/boost/include )
#link_directories ( /opt/boost/lib )
include_directories(${Boost_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR})

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

add_subdirectory(flagwld/base)
add_subdirectory(flagwld/net)
add_subdirectory(flagwld/utils)
