add_library(Console STATIC "Console.cpp" "Console.h" "Parsers.h" "Parsers.cpp")

target_link_libraries(Console "Util" "Indexing" "Exchanger")

target_include_directories(Console PUBLIC ".")
