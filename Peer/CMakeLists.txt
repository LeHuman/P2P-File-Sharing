add_library(Peer STATIC "Peer.cpp"  "Peer.h")

target_link_libraries(Peer "Util" "Indexing" "Console" "Exchanger")

target_include_directories(Peer PUBLIC ".")