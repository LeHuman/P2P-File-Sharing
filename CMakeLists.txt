﻿# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("P2P File Sharing")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (UNIX)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)

    link_libraries(Threads::Threads)
endif (UNIX)

set(RPCLIB_CXX_STANDARD "14")
set(BUILD_SHARED OFF)
set(BUILD_TESTING OFF)

# Include sub-projects.
add_subdirectory("asio/asio/include")

add_subdirectory("Exchanger")

add_subdirectory ("LocalTesting")

add_subdirectory("Util")

add_subdirectory("Indexing")

add_subdirectory("rpclib")

add_subdirectory("Peer")

add_subdirectory("Server")

add_subdirectory("Client")

add_subdirectory("Console")

add_subdirectory("cryptopp-cmake")

add_library(tclap INTERFACE)
target_include_directories(tclap INTERFACE "tclap/include")

add_subdirectory("TestPeer")
