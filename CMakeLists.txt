# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("P2P File Sharing")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include sub-projects.
add_subdirectory ("LocalTesting")

add_subdirectory("Util")

add_subdirectory("Indexing")

add_subdirectory("rpclib")

add_subdirectory("Peer")

add_subdirectory("Server")

add_subdirectory("Console")

add_subdirectory("PicoSHA2")
