add_library(Indexing STATIC   "indexRPC.cpp" "indexRPC.h" "index.cpp" "index.h")

target_link_libraries(Indexing "Util" "rpc")

target_include_directories(Indexing PRIVATE "${PROJECT_SOURCE_DIR}/Util")