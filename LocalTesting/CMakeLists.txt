# CMakeList.txt : CMake project for P2P File Sharing, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (LocalTesting "LocalTesting.cpp" )

target_link_libraries(LocalTesting "Util" "Indexing")

target_include_directories(LocalTesting PRIVATE "${PROJECT_SOURCE_DIR}/Util" "${PROJECT_SOURCE_DIR}/Indexing")

# TODO: Add tests and install targets if needed.
