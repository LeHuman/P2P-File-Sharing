add_library(Console STATIC "Console.cpp" "Console.h")

target_link_libraries(Console "Util" "Indexing")

target_include_directories(Console PRIVATE "${PROJECT_SOURCE_DIR}/Util" "${PROJECT_SOURCE_DIR}/Indexing")
