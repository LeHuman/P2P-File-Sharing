add_library(Exchanger STATIC "Exchanger.cpp" "Exchanger.h")

target_link_libraries(Exchanger PUBLIC "Util" "Indexing" "asio")

target_include_directories(Exchanger PUBLIC ".")